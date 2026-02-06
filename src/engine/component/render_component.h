#pragma once

namespace engine::component {
    struct RenderComponent {
        int layer_index_{0};    ///< 渲染层级索引，较大的值表示更靠前的层
        float y_index_{0.0f};  ///< y 轴深度索引，用于同一层内的渲染顺序控制

        bool operator<(const RenderComponent& other) const {
            if (layer_index_ != other.layer_index_) {
                return layer_index_ < other.layer_index_;
            }
            return y_index_ < other.y_index_;
        }
    };

}