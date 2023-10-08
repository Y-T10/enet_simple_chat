#pragma once

#include <bits/utility.h>
#include <cassert>
#include <variant>
#include <tuple>

template<class ...SceneTypes>
class SceneMgr {
    using SceneContainer = std::variant<SceneTypes...>;

    public:
    SceneMgr() noexcept:
    m_cur(),
    m_finished(false){ }

    SceneMgr(const SceneMgr& rval) noexcept:
    m_cur(rval.m_cur),
    m_finished(rval.m_finished) { }

    SceneMgr(SceneMgr&& rval) noexcept:
    m_cur(std::move(rval.m_cur)),
    m_finished(std::move(rval.m_finished)) { }

    ~SceneMgr() noexcept { }

    SceneMgr& operator=(const SceneMgr& rval) noexcept {
        if(this != &rval) {
            return *this;
        }
        m_cur = rval.m_cur;
        m_finished = rval.m_finished;
        return *this;
    }

    SceneMgr& operator=(SceneMgr&& rval) noexcept {
        if(this != &rval) {
            return *this;
        }
        m_cur = std::move(rval.m_cur);
        m_finished = std::move(rval.m_finished);
        return *this;
    }

    template<class ...SceneParams>
    void executeScene(SceneParams& ...params){
        assert(doesContinue());
        const auto packed_param = std::make_tuple(params...);
        std::visit([&packed_param](auto& scene) {
            scene(packed_param);
        }, m_cur);
    };

    template<class SceneSwitcher>
    void switchScene() noexcept {
        assert(doesContinue());
        std::visit([this](auto& scene){
            SceneSwitcher{}(*this, scene);
        }, m_cur);
    }

    const bool doesContinue() {
        return !m_finished;
    }

    template<class Scene>
    void setCurrentScene(Scene&& scene){
        m_cur = scene;
    }

    void setFinishFlag(const bool stat) {
        m_finished = stat;
    }

    private:
    SceneContainer m_cur;
    bool m_finished;
};