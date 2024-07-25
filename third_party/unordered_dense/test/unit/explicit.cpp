#include <ankerl/unordered_dense.h>

#include <app/doctest.h>

#include <unordered_map>
#include <vector>

struct texture {
    int m_width;
    int m_height;
    void* m_data;
};

struct per_image {
    std::unordered_map<void*, texture*> m_texture_index;
};

template <typename Map>
struct scene {
    std::vector<per_image> m_per_image;
    Map m_textures_per_key;
};

template <typename Map>
struct app_state {
    scene<Map> m_scene;
};

TYPE_TO_STRING_MAP(void*, texture*);

TEST_CASE_MAP("unit_create_AppState_issue_97", void*, texture*) {
    app_state<map_t> const app_state{};
}
