#define PN7160_INTERNAL
#include "pn7160_board_config.h"
#include "pn7160_board_defaults.h"
#include <string>
#include <mutex>
#include <cstring>
#include <cerrno>
#include <ctime>

namespace {
std::mutex config_mutex;
pn7160_config_t stored = {};
std::string device;
bool configured = false;
bool active = false;
std::string config_directory;
bool config_directory_locked = false;
}
extern "C" uint32_t pn7160_config_abi_version(void) { return PN7160_CONFIG_ABI_VERSION; }
static_assert(PN7160_DEFAULT_I2C_ADDRESS >= 0x08 && PN7160_DEFAULT_I2C_ADDRESS <= 0x77, "Invalid default I2C address");
static_assert(PN7160_DEFAULT_IRQ_GPIO >= 0 && PN7160_DEFAULT_IRQ_GPIO <= INT32_MAX, "Invalid default IRQ");
static_assert(PN7160_DEFAULT_VEN_GPIO >= 0 && PN7160_DEFAULT_VEN_GPIO <= INT32_MAX, "Invalid default VEN");
static_assert(PN7160_DEFAULT_DWL_REQ_GPIO >= -1 && PN7160_DEFAULT_DWL_REQ_GPIO <= INT32_MAX, "Invalid default DWL");
static_assert(PN7160_DEFAULT_IRQ_GPIO != PN7160_DEFAULT_VEN_GPIO &&
              PN7160_DEFAULT_IRQ_GPIO != PN7160_DEFAULT_DWL_REQ_GPIO &&
              PN7160_DEFAULT_VEN_GPIO != PN7160_DEFAULT_DWL_REQ_GPIO, "Conflicting default GPIOs");
static_assert(PN7160_DEFAULT_RESET_DELAY_US > 0 && PN7160_DEFAULT_RESET_DELAY_US <= UINT32_MAX, "Invalid default reset delay");
static_assert(PN7160_DEFAULT_I2C_DEVICE[0] == '/', "Default I2C path must be absolute");
extern "C" int pn7160_get_default_config(pn7160_config_t *cfg, uint32_t size) {
    if (!cfg || size < sizeof(*cfg)) return PN7160_CONFIG_INVALID;
    *cfg = {};
    cfg->abi_version = PN7160_CONFIG_ABI_VERSION;
    cfg->struct_size = sizeof(*cfg);
    cfg->i2c_device = PN7160_DEFAULT_I2C_DEVICE;
    cfg->i2c_address = PN7160_DEFAULT_I2C_ADDRESS;
    cfg->irq_gpio = PN7160_DEFAULT_IRQ_GPIO;
    cfg->ven_gpio = PN7160_DEFAULT_VEN_GPIO;
    cfg->dwl_req_gpio = PN7160_DEFAULT_DWL_REQ_GPIO;
    cfg->reset_delay_us = PN7160_DEFAULT_RESET_DELAY_US;
    return PN7160_CONFIG_OK;
}
extern "C" int pn7160_set_config(const pn7160_config_t *cfg) {
    if (!cfg) return PN7160_CONFIG_INVALID;
    if (cfg->abi_version != PN7160_CONFIG_ABI_VERSION || cfg->struct_size < sizeof(*cfg))
        return PN7160_CONFIG_ABI_MISMATCH;
    std::lock_guard<std::mutex> guard(config_mutex);
    if (active) return PN7160_CONFIG_BUSY;
    if (!cfg->i2c_device || cfg->i2c_device[0] != '/' || !cfg->reset_delay_us ||
        cfg->i2c_address < 0x08 || cfg->i2c_address > 0x77 ||
        cfg->irq_gpio < 0 || cfg->ven_gpio < 0 || cfg->dwl_req_gpio < -1 ||
        cfg->irq_gpio == cfg->ven_gpio || cfg->irq_gpio == cfg->dwl_req_gpio ||
        cfg->ven_gpio == cfg->dwl_req_gpio)
        return PN7160_CONFIG_INVALID;
    for (unsigned i=0; i<4; ++i) if (cfg->reserved[i]) return PN7160_CONFIG_INVALID;
    try {
        std::string copy(cfg->i2c_device);
        device.swap(copy);
    } catch (...) { return PN7160_CONFIG_INVALID; }
    stored = *cfg;
    stored.i2c_device = device.c_str();
    configured = true;
    return PN7160_CONFIG_OK;
}
extern "C" int pn7160_board_begin(void) {
    std::lock_guard<std::mutex> guard(config_mutex);
    if (!configured) return PN7160_CONFIG_MISSING;
    if (active) return PN7160_CONFIG_BUSY;
    active = true;
    return PN7160_CONFIG_OK;
}
extern "C" void pn7160_board_end(void) {
    std::lock_guard<std::mutex> guard(config_mutex);
    active = false;
}
extern "C" int pn7160_board_configured(void) { return configured ? 1 : 0; }
// These getters are used after begin() freezes the configuration.
extern "C" const char *pn7160_board_i2c_bus(void) { return stored.i2c_device; }
extern "C" uint32_t pn7160_board_i2c_address(void) { return stored.i2c_address; }
extern "C" int32_t pn7160_board_irq(void) { return stored.irq_gpio; }
extern "C" int32_t pn7160_board_ven(void) { return stored.ven_gpio; }
extern "C" int32_t pn7160_board_dwl(void) { return stored.dwl_req_gpio; }
extern "C" void pn7160_board_reset_wait(void) {
    struct timespec delay = {static_cast<time_t>(stored.reset_delay_us / 1000000u),
                            static_cast<long>((stored.reset_delay_us % 1000000u) * 1000u)};
    while (nanosleep(&delay, &delay) != 0 && errno == EINTR) {}
}

extern "C" int pn7160_set_config_directory(const char *path) {
    if (!path || path[0] != '/') return PN7160_CONFIG_INVALID;
    std::lock_guard<std::mutex> guard(config_mutex);
    if (active || config_directory_locked) return PN7160_CONFIG_BUSY;
    try {
        std::string copy(path);
        if (copy.back() != '/') copy += '/';
        config_directory.swap(copy);
    } catch (...) { return PN7160_CONFIG_INVALID; }
    return PN7160_CONFIG_OK;
}
extern "C" const char *pn7160_board_config_directory(void) {
    std::lock_guard<std::mutex> guard(config_mutex);
    config_directory_locked = true;
    return config_directory.empty() ? NULL : config_directory.c_str();
}
