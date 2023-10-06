#pragma once

#include <bits/utility.h>
#include <variant>
#include <tuple>

template<class ...SceneTypes>
class SceneMgr {
    public:
    class NullScene final {
        public:
        template <class Param>
        void operator()(const Param& param) { }
    };

    using SceneContainer = std::variant<NullScene, SceneTypes...>;

    public:
    SceneMgr() noexcept:
    m_cur(NullScene()){ }

    SceneMgr(const SceneMgr& rval) noexcept:
    m_cur(rval.m_cur) { }

    SceneMgr(SceneMgr&& rval) noexcept:
    m_cur(std::move(rval.m_cur)) { }

    ~SceneMgr() noexcept { }

    SceneMgr& operator=(const SceneMgr& rval) noexcept {
        if(this != &rval) {
            return *this;
        }
        m_cur = rval.m_cur;
        return *this;
    }

    SceneMgr& operator=(SceneMgr&& rval) noexcept {
        if(this != &rval) {
            return *this;
        }
        m_cur = std::move(rval.m_cur);
        return *this;
    }

    template<class ...SceneParams>
    void executeScene(SceneParams& ...params){
        const auto packed_param = std::make_tuple(*this, params...);
        std::visit([&packed_param](auto& scene) {
            scene(packed_param);
        }, m_cur);
    };

    const bool isQuit() {
        return std::holds_alternative<NullScene>(m_cur);
    }

    template<class Scene>
    void setCurrentScene(Scene&& scene){
        m_cur = scene;
    }

    private:
    SceneContainer m_cur;
};

template<class SceneParamsPacked, class Scene>
void SetCurrentScene(SceneParamsPacked& param, Scene&& scene) noexcept {
    auto mgr = std::get<0>(param);
    mgr.setCurrentScene(scene);
};

template<class SceneParamsPacked>
void ClearCurrentScene(SceneParamsPacked& param) noexcept {
    using SceneMgr = std::tuple_element_t<0, SceneParamsPacked>;
    auto mgr = std::get<0>(param);
    mgr.setCurrentScene(typename SceneMgr::NullScene());
};