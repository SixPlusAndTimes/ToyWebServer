//
// Created by 19269 on 2022/7/26.
//

#include "timer.h"

void TimerManager::handleExpireEvent() {
    if (heap_.empty())
    {
        return;
    }
    while (!heap_.empty())
    {   //循环执行过期的timer的回调函数
        // printf("m_heap不为空，正在处理过时连接\n");
        Timer node = heap_.front();
        int64_t resTime = std::chrono::duration_cast<_ms>(node.expire_ - _clock::now()).count();
        if ( resTime > 0)
        {
            //没有Timer过期，break
            // printf("当前连接还未超时,fd:%d, resTime:%ld\n", node.m_id, resTime);
            break;
        }
        //执行回调函数
        node.timeout_callback_();
        pop();
    }
}

void TimerManager::swapNode(int i, int j)
{
    // LOG_DEBUG(" ");
//    heap_mutex_.lock();
    std::swap(heap_[i], heap_[j]);
//    hash_table_mutex_.lock();
    fd_id_map_[heap_[i].so_fd_] = i;
    fd_id_map_[heap_[j].so_fd_] = j;
//    hash_table_mutex_.unlock();
//    heap_mutex_.lock();
}
void TimerManager::swim(int index) {
    if(index < 0 || index >= static_cast<int>(heap_.size()) ) {
        LOG_DEBUG("invalid index of heap");
    }

    if(index == 0) return;
    int parent_index = (index - 1) / 2;
//    std::cout <<"parent index = "<< parent_index <<std::endl;
//    std::cout << "heap[parent_index] = " << heap_[parent_index].expire_.time_since_epoch().count() <<std::endl;
//    std::cout << "heap[index] = " << heap_[index].expire_.time_since_epoch().count() <<std::endl;
//    std::cout << (heap_[parent_index] < heap_[index]) << std::endl;

    while(parent_index >= 0) {
        if(heap_[parent_index] < heap_[index]) {
            //如果 父节点的过期时间小于新的节点的过期时间，那么就说明此时整棵树已维持了最小对的性质
//            std::cout << "heap_[parent_index] < heap_[index]" <<std::endl;
            break;
        }
        //交换父子节点
        swapNode(index, parent_index);
        //重复交换的过程
        index = parent_index;
        parent_index = (index - 1) / 2;
    }
}



void TimerManager::m_del(int i) {
    // 删除指定小根堆中指定的节点
    // std::cout << "TimeManager::m_del(int i)" << i  << std::endl;
    if (i < 0 || i > static_cast<int>(heap_.size()))
    {
        LOG_DEBUG("invalid i:%d", i);
        return;
    }
    int j = i; // 要删除的结点坐标
    int n = heap_.size() - 1; // 最后一个结点的index
    // LOG_DEBUG("j:%d, n:%d", j, n);
    if (j < n)
    {
        //将要删除的元素换到队尾，然后用下沉或上浮算法调整
        swapNode(j, n);
        if (!sink(j, n))
        {
            //如果下沉算法没有执行成功，那么执行上浮算法
            swim(j);
        }
    }
    /* 删除队尾元素 */
    // LOG_DEBUG("fd:%d",m_heap[n].m_id)
    fd_id_map_.erase(heap_[n].so_fd_);
    // LOG_DEBUG("删除之前m_heap大小:%d", m_heap.size());
    heap_.pop_back();
    // LOG_DEBUG("删除之后m_heap大小:%d", m_heap.size());
}


bool TimerManager::sink(int i, int n) {
    if (i < 0 || i >= static_cast<int>(heap_.size()))
    {
        LOG_DEBUG("invalid i:%d", i);
        return false;
    }
    if (n < 0 || n > static_cast<int>(heap_.size()))
    {
        LOG_DEBUG("invalid n");
        return false;
    }
    int j = i;
    int k = j * 2 + 1;
    while (k < n)
    {
        if (k + 1 < n && heap_[k + 1] < heap_[k])
        {
            k++;
        }
        if (heap_[j] < heap_[k])
        {
            break;
        }
        swapNode(j, k);
        j = k;
        k = j * 2 + 1;
    }
    // 如果j > i ,表示确实有节点下移了，表示下沉算法执行成功
    return j > i;
}
void TimerManager::addTimer(int fd, int timeout, const _timeoutCallBack &func) {
    if(fd < 0) {
        LOG_WARN("invalid fd");
        return;
    }
    int index_in_heap;
    if(fd_id_map_.count(fd) == 0) {
        //如果加入的是新的Timer，即fd是新的
        //这一步保证树是完全二叉树
//        std::cout << "no exits fd: " << fd << std::endl;
        index_in_heap = heap_.size();

        fd_id_map_[fd] = index_in_heap;

        heap_.push_back(Timer(fd, _clock::now() + _ms(timeout), func));
//        std::cout<<" index in heap = " << index_in_heap << std::endl;
        swim(index_in_heap);//插入后跟新最小堆

//        std::cout << "fd_id_map : \n";
//        for(auto elem : fd_id_map_) {
//            std::cout << "\tfd = " << elem.first << ": index = " <<elem.second <<std::endl;
//        }
//        std::cout << "heap : \n";
//        for(auto ele : heap_) {
//            std::cout << ele.expire_.time_since_epoch().count() << "  ";
//        }
//        std::cout << "\n";
    }else {
        //如果heap中已经存在了这个timer对应的fd
//        std::cout << "has exits fd: " << fd << std::endl;
        index_in_heap = fd_id_map_[fd];
        heap_[index_in_heap].expire_ = _clock ::now() + _ms(timeout);
        heap_[index_in_heap].timeout_callback_ = func;

        if(!sink(index_in_heap, heap_.size()) ){
            //先下移节点，找到合适节点
            //如果没找到，那就使用上移算法
            swim(index_in_heap);
        }
    }
}
// 触发指定index 的timer回调函数，并将它删除
void TimerManager::work(int id) {
    /* 删除指定id结点，并触发回调函数 */
    //std::cout << "TimeManager::work(int id)" << std::endl;
    if (heap_.empty() || fd_id_map_.count(id) == 0)
    {
        LOG_DEBUG("invalid id or empty heap");
        return;
    }
    int i = fd_id_map_[id];
    Timer node = std::move(heap_[i]);
    //执行回调函数
    node.timeout_callback_();
    //删除这个节点
    m_del(i);
}

void TimerManager::delFd(int fd) {
    if(fd_id_map_.count(fd))
    {
        int pos = fd_id_map_[fd];
        // LOG_DEBUG("input fd: %d, fd's pos: %d", fd, pos);
        m_del(pos);
    }
    else
    {
        LOG_ERROR("m_ref do not have fd:%d",fd);
        return;
    }
}

//首先调用handleExpireEvent()将到期的timer的回调函数全部执行
    //然后返回heap中的最顶端timer距离它的过期时间还有多长时间
int TimerManager::getNextHandle()
{
    // 先去小根堆看一下有没有过期的socket通信。如果有，关闭该socket通信
    handleExpireEvent();
    int64_t res = -1;
    if (!heap_.empty())
    {
        // 获取小根堆中第一个还没有过期的结点的时间，单位是ms
        res = std::chrono::duration_cast<_ms>(heap_.front().expire_ - _clock::now()).count();
        if (res < 0)
        {
            // 为什么会出现这种情况？调用完handleExpiredEvent清理掉了已经超时的连接，根节点的连接，
            // 程序运行到这也是需要一段时间的（几ms的可能性），说不定恰好在这就超时了
            res = 0;
        }
    }
    return res;
}

void TimerManager::updateTimer(int fd, int timeout) {
    /*由于客户端的浏览器会在得到访问资源后，主动断开，所以这个if检查是很有必要的*/
//    LOG_DEBUG("[%d]进入",id);
    if (heap_.empty())
    {
        LOG_DEBUG("empty heap");
        return;
    }
    if (fd_id_map_.count(fd) == 0)
    {
        LOG_DEBUG("invalid fd:%d", fd);
        return;
    }
    int i = fd_id_map_[fd];
    heap_[i].expire_ = _clock::now() + _ms(timeout);
    if (!sink(i, heap_.size()))
    {
        swim(i);
    }
}

void TimerManager::pop() {
    if (heap_.empty())
    {
        LOG_DEBUG("empty heap");
        return;
    }
    m_del(0);
}
void TimerManager::clear()
{
    heap_.clear();
    fd_id_map_.clear();
}