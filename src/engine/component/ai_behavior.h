#pragma once
#include <glm/vec2.hpp>
namespace engine {
    namespace object {
        class GameObject;
    }
    namespace core {
        class Context;
    }
}

namespace engine::component {

    /**
     * @class AIBehavior
     * @brief AI行为的抽象基类，定义策略接口
     * 
     * 所有具体的AI行为类都继承自这个基类，实现不同的行为逻辑
     */
    class AIBehavior {
    public:
        virtual ~AIBehavior() = default;
        
        /**
         * @brief 更新AI行为
         * 
         * @param owner 拥有该行为的游戏对象
         * @param deltaTime 时间增量
         * @param context 引擎上下文
         */
        virtual void update(engine::object::GameObject* owner, float deltaTime, engine::core::Context& context) = 0;
        
        /**
         * @brief 初始化行为
         * 
         * @param owner 拥有该行为的游戏对象
         */
        virtual void init(engine::object::GameObject* /*owner*/) {}
        
        /**
         * @brief 清理行为
         * 
         * 用于在游戏对象销毁时清理资源
         */
        virtual void clean() {}
    };

}  // namespace engine::component
