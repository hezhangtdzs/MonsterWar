#pragma once
#include "../render/sprite.h"
#include "./component.h"
#include "../utils/alignment.h"
#include <string>
#include <optional>
#include <SDL3/SDL_rect.h>
#include <glm/vec2.hpp>

namespace engine::render {
	class Sprite;
}
namespace engine::utils {
	enum class Alignment;
}
namespace engine::resource
{
	class ResourceManager;
}

namespace engine::component {
	class TransformComponent;

	/**
	 * @class SpriteComponent
	 * @brief 精灵组件类，负责处理游戏对象的 2D 图像渲染。
	 * 
	 * 该组件封装了 Sprite 对象，并提供对齐、偏移和可见性控制。
	 * 它与 TransformComponent 配合使用，以确定渲染位置。
	 */
	class SpriteComponent final : public Component {
		friend class engine::object::GameObject;
	private:
		engine::resource::ResourceManager* resource_manager_ = nullptr;         ///< @brief 保存资源管理器指针，用于获取纹理大小
		TransformComponent* transform_ = nullptr;                               ///< @brief 缓存 TransformComponent 指针（非必须）

		engine::render::Sprite sprite_;                                         ///< @brief 精灵对象
		engine::utils::Alignment alignment_ = engine::utils::Alignment::NONE;   ///< @brief 对齐方式
		glm::vec2 sprite_size_ = { 0.0f, 0.0f };                                  ///< @brief 精灵尺寸
		glm::vec2 offset_ = { 0.0f, 0.0f };                                       ///< @brief 偏移量
		bool is_hidden_ = false;                                                ///< @brief 是否隐藏
	public:
		/**
		 * @brief 构造一个新的 SpriteComponent。
		 * @param texture_id 纹理资源的唯一标识符。
		 * @param resource_manager 资源管理器引用，用于加载和查询纹理。
		 * @param alignment 精灵相对于变换位置的对齐方式，默认为 NONE。
		 * @param source_rect_opt 可选的源矩形，用于指定渲染纹理的特定区域（如精灵图帧）。
		 * @param is_flipped 是否水平翻转精灵。
		 */
		SpriteComponent(
			const std::string& texture_id,
			engine::resource::ResourceManager& resource_manager,
			engine::utils::Alignment alignment = engine::utils::Alignment::NONE,
			std::optional<SDL_FRect> source_rect_opt = std::nullopt,
			bool is_flipped = false
		);
		SpriteComponent(
			engine::render::Sprite&& sprite,
			engine::resource::ResourceManager& resource_manager,
			engine::utils::Alignment alignment = engine::utils::Alignment::NONE
		);
		~SpriteComponent() = default;

		// 禁止拷贝和移动语义
		SpriteComponent(const SpriteComponent&) = delete;
		SpriteComponent& operator=(const SpriteComponent&) = delete;
		SpriteComponent(SpriteComponent&&) = delete;
		SpriteComponent& operator=(SpriteComponent&&) = delete;
		
		/**
		 * @brief 根据当前的对齐方式和精灵尺寸更新偏移量。
		 */
		void updateOffset();

		// Getters
		const engine::render::Sprite& getSprite() const { return sprite_; }         ///< @brief 获取精灵对象
		const std::string& getTextureId() const { return sprite_.getTextureId(); }  ///< @brief 获取纹理ID
		bool isFlipped() const { return sprite_.getIsFlipped(); }                      ///< @brief 获取是否翻转
		bool isHidden() const { return is_hidden_; }                        ///< @brief 获取是否隐藏
		const glm::vec2& getSpriteSize() const { return sprite_size_; }             ///< @brief 获取精灵尺寸
		const glm::vec2& getOffset() const { return offset_; }                      ///< @brief 获取偏移量
		engine::utils::Alignment getAlignment() const { return alignment_; }        ///< @brief 获取对齐方式

		// Setters
		/**
		 * @brief 通过纹理 ID 更改精灵的纹理。
		 * @param texture_id 新的纹理 ID。
		 * @param source_rect_opt 新的可选源矩形。
		 */
		void setSpriteById(const std::string& texture_id, const std::optional<SDL_FRect>& source_rect_opt = std::nullopt);
		
		void setFlipped(bool flipped) { sprite_.setIsFlipped(flipped); }           ///< @brief 设置是否翻转
		void setHidden(bool hidden) { is_hidden_ = hidden; }                       ///< @brief 设置是否隐藏
		
		/**
		 * @brief 设置精灵的源矩形。
		 * @param source_rect_opt 源矩形，为 std::nullopt 时渲染整个纹理。
		 */
		void setSourceRect(const std::optional<SDL_FRect>& source_rect_opt);
		
		/**
		 * @brief 设置精灵的对齐方式并更新偏移。
		 * @param anchor 新的对齐锚点。
		 */
		void setAlignment(engine::utils::Alignment anchor);

	private:
		/**
		 * @brief 更新精灵的逻辑尺寸属性。
		 */
		void updateSpriteSize();

		/**
		 * @brief 初始化组件，获取所需的其他组件引用。
		 */
		void init() override;

		/**
		 * @brief 每帧更新逻辑。
		 * @param deltaTime 每帧的时间间隔。
		 * @param context 引擎上下文环境。
		 */
		void update(float deltaTime, engine::core::Context& context) override;

		/**
		 * @brief 执行渲染操作。
		 * @param context 引擎上下文环境。
		 */
		void render(engine::core::Context& context) override;
	};
}