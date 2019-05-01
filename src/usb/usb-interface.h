// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#pragma once

#include "usb-types.h"
#include "usb-endpoint.h"
#include "usb-messenger.h"

#include <memory>
#include <vector>

namespace librealsense
{
    namespace platform
    {
        class usb_interface
        {
        public:
            usb_interface() = default;
            virtual ~usb_interface() = default;

            virtual uint8_t get_number() const = 0;
            virtual usb_subclass get_subclass() const = 0;
            virtual const std::vector<std::shared_ptr<usb_endpoint>> get_endpoints() const = 0;

            virtual const rs_usb_endpoint first_endpoint(endpoint_direction direction, endpoint_type type = USB_ENDPOINT_BULK) const = 0;
        };

        typedef std::shared_ptr<usb_interface> rs_usb_interface;
    }
}