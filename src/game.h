#ifndef GAME_GAME
#define GAME_GAME

#include "sgl.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct Player {
    uint32_t color;
    float vel;
    float angle;
    float acc;
    float timeout;
    float health;
    float driven_distance;
    float old_driven_distance;
} Player;
#define PLAYER_COMPONENT (1 << 0)

typedef struct Position {
    float x;
    float y;
} Position;
#define POSITION_COMPONENT (1 << 1)

typedef struct Velocity {
    float vel;
} Velocity;
#define VELOCITY_COMPONENT (1 << 2)

typedef struct Acceleration {
    float acc;
} Acceleration;
#define ACCELERATION_COMPONENT (1 << 3)

typedef struct Direction {
    float angle;
} Direction;
#define DIRECTION_COMPONENT (1 << 4)

typedef struct CircleCollider {
    float radius;
    float mass;
} CircleCollider;
#define CIRCLE_COLLIDER_COMPONENT (1 << 5)

typedef struct CapsuleCollider {
    float radius;
    float sx;
    float sy;
    float ex;
    float ey;
} CapsuleCollider;
#define CAPSULE_COLLIDER_COMPONENT (1 << 6)

typedef struct Projectile {
    uint32_t entity_source;
    uint32_t color;
} Projectile;
#define PROJECTILE_COMPONENT (1 << 7)

typedef struct TimeToLive {
    float time;
} TimeToLive;
#define TIME_TO_LIVE_COMPONENT (1 << 8)

#define appearance_func(name) void name(uint32_t id)
typedef appearance_func(appearance_f);
typedef struct Appearance {
    appearance_f* draw;
    uint16_t layer;
} Appearance;
#define APPEARANCE_COMPONENT (1 << 9)

// #define MAX_ENTITIES (1024 * 63)
#define MAX_ENTITIES (240 * 4)
#define ENTITY_DEAD 0
#define ENTITY_ALIVE 1

typedef struct QueryResult {
    uint32_t* ids;
    uint32_t num_results;
} QueryResult;

typedef struct Mouse {
    int x;
    int y;
    bool left;
    bool right;
} Mouse;

typedef struct World {
    sglFont* font;

    sglBuffer* buffer;
    sglBuffer* debug_buffer;
    const uint8_t* keyboard;
    Mouse m;
    float time_since_start;

    Player* player_components;
    Position* position_components;
    Velocity* velocity_components;
    Acceleration* acceleration_components;
    Direction* direction_components;
    CircleCollider* circle_collider_components;
    CapsuleCollider* capsule_collider_components;
    Projectile* projectile_components;
    TimeToLive* time_to_live_components;
    Appearance* appearance_components;
    uint32_t* component_masks;

    // NOTE: 32 bits not necessary as it will never exceed MAX_ENTITIES, which
    // is 1024*63
    uint32_t* ids;
    uint32_t next_id;
    uint32_t* entity_states;
    uint32_t* queried_ids;
    uint32_t query_num_results;
	QueryResult queries[4];

    uint32_t player_ids[4];
    // uint32_t obstacle_ids[32];

    bool debug_mode;
} World;

void init_ecs(void);
void reset_world(void);
void init_game(void);
void inject_code_on_reload(void);
void step(void);

uint32_t create_entity(void);
void delete_entity(uint32_t id);
void add_component(uint32_t id, uint32_t component);
void remove_component(uint32_t id, uint32_t component);
bool has_component(uint32_t id, uint32_t component);
void query_entities(QueryResult* qr, uint32_t query_component_mask);

void handleInput(void);
void draw_system(void);
void physics_system(void);
void time_to_live_system(void);

typedef enum Operation {
    OP_LOAD,
    OP_INIT,
    OP_INJECT,
    OP_STEP,

} Operation;

#define GAME_MAIN(name) int name(World* w, Operation op)
typedef GAME_MAIN(game_main_f);
GAME_MAIN(game_main);

#endif
