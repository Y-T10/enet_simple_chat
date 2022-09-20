#include"sig_event.hpp"

#include<cassert>
#include<cerrno>

SigEvent::SigEvent() noexcept
:m_sigMask(){};

const bool SigEvent::set_signal(const std::vector<int>& sigs){
    //シグナルのマスクを初期化する
    sigemptyset(&m_sigMask);
    const auto func = [this](const int sig){
        assert(sig > 0);
        if(sig == SIGKILL || sig == SIGSTOP){
            return;
        }
        sigaddset(&m_sigMask, sig);
    };
    //シグナルを設定する
    std::ranges::for_each(sigs, func);
    //指定したシグナルの関数が呼ばれないようにする
    return sigprocmask(SIG_BLOCK, &m_sigMask, NULL) != -1;
}

void SigEvent::handle_signal(const std::function<void(const int)>& handler){
    //すぐ返す
    constexpr struct timespec wait_time = { .tv_sec = 0, .tv_nsec = 0 };
    //シグナルを取り出す
    while(true){
        const int signal = sigtimedwait(&m_sigMask, NULL, &wait_time);
        //監視対象のシグナルがあるか
        if(signal > 0){
            handler(signal);
            continue;
        }
        //エラー番号を取り出す
        const int last_error = errno;
        assert(last_error != EINVAL);
        //もうシグナルがないかを判断する
        if(last_error == EAGAIN){
            return;
        }
    }
}