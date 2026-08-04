/*
 * Copyright (C) 2026 The WitAqua Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>

#include "sku.h"

/*
 * Default for the sm8250 devices: take the model name from LTALabel and leave
 * everything else at whatever build.prop says.
 *
 * A device that also wants to look like its japanese variant - matching brand,
 * name and build fingerprint - provides its own libinit and points
 * soong_config_set(libinit, vendor_init_lib) at it instead. See
 * device/sony/pdx203/libinit.
 */
void vendor_load_properties() {
    std::string model = sony::DetectLtaModel();
    if (model.empty()) {
        LOG(INFO) << "libinit: no japanese model in LTALabel, keeping the built-in model";
        return;
    }

    LOG(INFO) << "libinit: detected " << model;
    sony::SetModelProperties(model);
}
