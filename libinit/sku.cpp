/*
 * Copyright (C) 2026 The WitAqua Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sku.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <string_view>
#include <vector>

#include <android-base/logging.h>

// Not exported by system/core/init, but it has external linkage and this ends
// up linked into libinit itself.
namespace android {
namespace init {
uint32_t InitPropertySet(const std::string& name, const std::string& value);
}  // namespace init
}  // namespace android

namespace sony {

namespace {

constexpr char kLtaLabel[] = "/dev/block/by-name/LTALabel";

/*
 * LTALabel holds the regulatory label as an HTML page, in which the marketing
 * name of the device appears as "<model>&nbsp;". These are the same names
 * init.qcom.msim.sh looks for. Only the japanese models are listed: everything
 * else keeps the PRODUCT_MODEL baked into build.prop.
 */
constexpr const char* kModels[] = {
        "A002SO", "SOG01", "SOG02", "SO-51A", "SO-52A",
};

}  // namespace

std::string DetectLtaModel() {
    int fd = TEMP_FAILURE_RETRY(open(kLtaLabel, O_RDONLY | O_CLOEXEC));
    if (fd < 0) {
        PLOG(WARNING) << "libinit: cannot open " << kLtaLabel;
        return {};
    }

    // The label sits a few MB into the partition, so stream through it instead
    // of reading all 16MB at once. The overlap keeps a match from being split
    // across two reads.
    constexpr size_t kChunkSize = 1 << 20;
    constexpr size_t kOverlap = 32;
    std::vector<char> buffer(kChunkSize + kOverlap);
    size_t carried = 0;
    std::string found;

    while (found.empty()) {
        ssize_t n = TEMP_FAILURE_RETRY(read(fd, buffer.data() + carried, kChunkSize));
        if (n <= 0) break;

        std::string_view haystack(buffer.data(), carried + n);
        for (const char* model : kModels) {
            if (haystack.find(std::string(model) + "&nbsp;") != std::string_view::npos) {
                found = model;
                break;
            }
        }

        carried = std::min(kOverlap, haystack.size());
        memmove(buffer.data(), buffer.data() + haystack.size() - carried, carried);
    }

    close(fd);
    return found;
}

void SetProperty(const std::string& name, const std::string& value) {
    android::init::InitPropertySet(name, value);
}

void SetModelProperties(const std::string& model) {
    SetProperty("ro.product.model", model);
    for (const char* partition : {"odm", "product", "system", "system_ext", "vendor"}) {
        SetProperty(std::string("ro.product.") + partition + ".model", model);
    }
}

}  // namespace sony
