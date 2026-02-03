#pragma once
#include "component.h"
#include "ai_behavior.h"
#include <memory>

namespace engine::component {

    /**
     * @class AIComponent
     * @brief AI组件，作为上下文持有并调用策略对象
     * 
     * 该组件负责管理AI行为，持有一个AIBehavior的实例，并在每帧调用其update方法
     */
    class AIComponent final : public Component {
        friend class engine::object::GameObject;
    private:
        std::unique_ptr<AIBehavior> behavior_; ///< 指向当前AI行为的指针
        
    public:
        /**
         * @brief 构造函数
         * 
         * @param behavior AI行为实例
         */
        explicit AIComponent(std::unique_ptr<AIBehavior> behavior);
        ~AIComponent() override = default;
        
        AIComponent(const AIComponent&) = delete;
        AIComponent& operator=(const AIComponent&) = delete;
        AIComponent(AIComponent&&) = delete;
        AIComponent& operator=(AIComponent&&) = delete;
        
        /**
         * @brief 设置AI行为
         * 
         * @param behavior 新的AI行为实例
         */
        void setBehavior(std::unique_ptr<AIBehavior> behavior);
        
        /**
         * @brief 获取当前AI行为
         * 
         * @return 当前AI行为的指针
         */
        AIBehavior* getBehavior() const;
        
    private:
        void init() override;
        void update(float deltaTime, engine::core::Context& context) override;
        void clean() override;
    };

}  // namespace engine::component
