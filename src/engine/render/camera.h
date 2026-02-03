#pragma once
#include "../utils/math.h"
#include <optional>
#include <spdlog/spdlog.h>

namespace engine::component {
	class TransformComponent;
}

/**
 * @namespace engine::render
 * @brief 包含所有与渲染相关的引擎组件。
 */
namespace engine::render {
	/**
	 * @class Camera
	 * @brief 摄像机类，负责视口管理、坐标转换和边界平滑移动。
	 * 
	 * 该类不可被拷贝或移动，确保摄像机实例的唯一性。
	 */
	class Camera final {
	private:
		/// 视口的尺寸 (宽度, 高度)
		glm::vec2 viewport_size_;
		/// 摄像机在世界空间中的中心坐标
		glm::vec2 position_;
		/// 摄像机的可选移动边界限制
		std::optional<engine::utils::Rect> limit_bounds_;
		float smooth_speed_ = 5.0f;
		engine::component::TransformComponent* target_ = nullptr; // 跟随目标
		bool pixel_snap_{ true };

	public:
		/**
		 * @brief 构造一个新的 Camera 对象。
		 * @param viewport_size 初始视口尺寸。
		 * @param position 初始中心位置。
		 * @param limit_bounds 可选的限制边界。
		 */
		explicit Camera(const glm::vec2& viewport_size,
						const glm::vec2& position = {0.0f, 0.0f},
						const std::optional<engine::utils::Rect>& limit_bounds = std::nullopt)
						:viewport_size_(viewport_size), position_(position), limit_bounds_(limit_bounds) {
			spdlog::trace("Camera 初始化成功，位置: {},{}", position_.x, position_.y);
		}

		/**
		 * @brief 析构摄像机对象。
		 */
		~Camera();

		/**
		 * @brief 每帧更新摄像机的状态。
		 * @param delta_time 上一帧到当前帧的时间偏差。
		 */
		void update(float delta_time);

		/**
		 * @brief 移动摄像机一段相对位移。
		 * @param offset 要移动的偏移量。
		 */
		void move(const glm::vec2& offset);

		/**
		 * @brief 将世界坐标映射到当前视口的屏幕坐标。
		 * @param world_pos 目标世界位置。
		 * @return glm::vec2 对应的屏幕位置。
		 */
		glm::vec2 worldToScreen(const glm::vec2& world_pos) const;

		/**
		 * @brief 将带视差的世界坐标映射到屏幕坐标。
		 * @param world_pos 目标世界位置。
		 * @param scroll_factor 滚动系数，值越小背景移动越慢。
		 * @return glm::vec2 对应的屏幕位置。
		 */
		glm::vec2 worldToScreenWithParallax(const glm::vec2& world_pos, float scroll_factor) const;

		/**
		 * @brief 将带视差的世界坐标映射到屏幕坐标 (X/Y 独立滚动系数)。
		 * @param world_pos 目标世界位置。
		 * @param scroll_factor 滚动系数，值越小背景移动越慢。
		 * @return glm::vec2 对应的屏幕位置。
		 */
		glm::vec2 worldToScreenWithParallax(const glm::vec2& world_pos, const glm::vec2& scroll_factor) const;

		/**
		 * @brief 将屏幕坐标映射回世界坐标。
		 * @param screen_pos 视口内的屏幕位置。
		 * @return glm::vec2 对应的世界位置。
		 */
		glm::vec2 screenToWorld(const glm::vec2& screen_pos) const;

		void setPixelSnap(bool enabled) { pixel_snap_ = enabled; }
		bool getPixelSnap() const { return pixel_snap_; }

		/**
		 * @brief 直接设置摄像机的世界坐标。
		 * @param position 新的位置中心点。
		 */
		void setPosition(const glm::vec2& position);

		/**
		 * @brief 设置摄像机的可移动范围。
		 * @param bounds 世界空间下的矩形边界。
		 */
		void setLimitBounds(const engine::utils::Rect& bounds);

		/**
		 * @brief 获取摄像机的当前位置。
		 * @return const glm::vec2& 分量形式的位置引用。
		 */
		const glm::vec2& getPosition() const;

		/**
		 * @brief 获取当前的移动限制边界（如果有）。
		 * @return std::optional<engine::utils::Rect> 配置的边界矩形。
		 */
		std::optional<engine::utils::Rect> getLimitBounds() const;

		/**
		 * @brief 获取摄像机视口的尺寸。
		 * @return glm::vec2 (宽, 高)。
		 */
		glm::vec2 getViewportSize() const;

		/**
		 * @brief 设置摄像机的跟随目标。
		 * @param target 要跟随的 Transform 组件指针。
		 */
		void setTarget(engine::component::TransformComponent* target);
		/**
		 * @brief 获取当前的跟随目标。
		 * @return engine::component::TransformComponent* 跟随目标指针。
		 */
		engine::component::TransformComponent* getTarget() const;
		/**
		 * @brief 设置摄像机的平滑移动速度。
		 * @param speed 平滑速度系数。
		 */
		void setSmoothSpeed(float speed);
		/**
		 * @brief 获取摄像机的平滑移动速度。
		 * @return float 平滑速度系数。
		 */
		float getSmoothSpeed() const;


		// 禁用拷贝和移动语义以防止对象不一致
		Camera(const Camera&) = delete;
		Camera& operator=(const Camera&) = delete;
		Camera(Camera&&) = delete;
		Camera& operator=(Camera&&) = delete;

	private:
		/**
		 * @brief 内部方法：应用 limit_bounds_ 限制摄像机的 position_。
		 */
		void clampPosition();
	};
}