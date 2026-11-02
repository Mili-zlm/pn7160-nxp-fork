#include "pn7160_board_config.h"
int main() {
    pn7160_config_t c;
    return pn7160_get_default_config(&c, sizeof(c));
}
