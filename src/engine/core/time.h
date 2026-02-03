#pragma once
#include <SDL3/SDL_stdinc.h> 

namespace engine::core {
    /**
     * @brief 基础时间管理类，负责计算 Delta Time、管理时间缩放以及帧率限制。
     */
    class Time final {
    private:
        Uint64 last_time_ = 0;           ///< 上一帧更新时的时间戳
        Uint64 frame_start_time_ = 0;    ///< 当前帧开始时的原始时间戳
        double delta_time_ = 0.0;        ///< 两帧之间的真实时间间隔（秒）
        double time_scale_ = 1.0;        ///< 时间缩放比例（例如 0.5 为慢动作）

        int target_fps_ = 0;             ///< 期望的目标帧率
        double target_frame_time_ = 0.0; ///< 目标帧率对应的每帧时长
    public:
        /**
         * @brief 构造函数，初始化计时数据。
         */
		Time();

        // 禁用拷贝和移动，确保单例或受控生命周期
        Time(const Time&) = delete;
        Time& operator=(const Time&) = delete;
        Time(Time&&) = delete;
        Time& operator=(Time&&) = delete;

        /**
         * @brief 在主循环每帧开始时更新时间状态。
         * 计算自上一帧以来的 delta time，并处理帧率限制。
         */
		void update();

        /**
         * @brief 获取当前帧的原始 Delta Time（未经缩放）。
         * @return float 秒为单位的时间间隔。
         */
		float getDeltaTime() const { return static_cast<float>(delta_time_); }

        /**
         * @brief 获取考虑时间缩放后的 Delta Time。
         * 常用于基于时间的物理移动或动画更新。
         * @return float 缩放后的秒数。
         */
		float getScaledDeltaTime() const { return static_cast<float>(delta_time_ * time_scale_); }

        /**
         * @brief 设置全局时间缩放系数。
         * @param scale 缩放倍率（1.0 为正常速度）。
         */
        void setTimeScale(double scale);

        /**
         * @brief 获取当前的时间缩放系数。
         * @return float 当前缩放值。
         */
		float getTimeScale() const { return static_cast<float>(time_scale_); }

        /**
         * @brief 设置游戏运行的目标 FPS。
         * @param fps 目标帧率（如 60），设为 0 表示不限制。
         */
		void setTargetFPS(int fps);

        /**
         * @brief 获取当前设定的目标帧率。
         * @return int 目标 FPS 值。
         */
		int getTargetFPS() const { return target_fps_; }

	private:
        /**
         * @brief 内部辅助方法，若实际运行速度过快则强制延迟以维持目标帧率。
         * @param current_delta_time 当前帧处理完毕实际耗费的时间。
         */
		void limitFrameRate(float current_delta_time);
    };
}