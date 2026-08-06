/*
 * Copyright (C) 2026 The WitAqua Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/strings.h>

#include <string>
#include <string_view>

namespace android {
namespace base {

/*
 * ese_spi_nxp.so was built against the libbase of Android 12, where Trim() was
 * a plain function taking a const std::string&. It is a template over
 * string-likes now, so the old mangled name is gone and the stock NFC and eSE
 * HALs fail to link:
 *
 *   CANNOT LINK EXECUTABLE ".../android.hardware.secure_element@1.2-service":
 *   cannot locate symbol "_ZN7android4base4TrimERKNSt3__1...E"
 *   referenced by "/vendor/lib64/ese_spi_nxp.so"
 *
 * Defining it non-template reproduces exactly that symbol. The call below
 * picks the template - the string_view prvalue binds to T&& without a
 * conversion, so this does not recurse.
 */
std::string Trim(const std::string& s) {
    return Trim(std::string_view(s));
}

}  // namespace base
}  // namespace android
