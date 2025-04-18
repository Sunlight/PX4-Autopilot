/****************************************************************************
 *
 *   Copyright (c) 2020 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef MANUAL_CONTROL_HPP
#define MANUAL_CONTROL_HPP

#include <uORB/topics/manual_control_setpoint.h>
#include <uORB/topics/manual_control_switches.h>

class MavlinkStreamManualControl : public MavlinkStream
{
public:
	static MavlinkStream *new_instance(Mavlink *mavlink) { return new MavlinkStreamManualControl(mavlink); }

	static constexpr const char *get_name_static() { return "MANUAL_CONTROL"; }
	static constexpr uint16_t get_id_static() { return MAVLINK_MSG_ID_MANUAL_CONTROL; }

	const char *get_name() const override { return get_name_static(); }
	uint16_t get_id() override { return get_id_static(); }

	unsigned get_size() override
	{
		return _manual_control_setpoint_sub.advertised() ? (MAVLINK_MSG_ID_MANUAL_CONTROL_LEN + MAVLINK_NUM_NON_PAYLOAD_BYTES) :
		       0;
	}

private:
	explicit MavlinkStreamManualControl(Mavlink *mavlink) : MavlinkStream(mavlink) {}

	uORB::Subscription _manual_control_setpoint_sub{ORB_ID(manual_control_setpoint)};
	uORB::Subscription _manual_control_switches_sub{ORB_ID(manual_control_switches)};

	bool send() override
	{
		manual_control_setpoint_s manual_control_setpoint;

		if (_manual_control_setpoint_sub.update(&manual_control_setpoint)) {
			mavlink_manual_control_t msg{};

			msg.target = mavlink_system.sysid;
			msg.x = manual_control_setpoint.pitch * 1000.f;
			msg.y = manual_control_setpoint.roll * 1000.f;
			// Convert from internal range [-1, 1] to [0, 1000]
			// ([-1, 1] + 1) / 2 * 1000 = [0, 1000]
			msg.z = (manual_control_setpoint.throttle + 1.0f) * 500.0f;
			msg.r = manual_control_setpoint.yaw * 1000.f;

			manual_control_switches_s manual_control_switches{};

			if (_manual_control_switches_sub.copy(&manual_control_switches)) {
				unsigned shift = 2;
				msg.buttons = 0;
				msg.buttons |= (manual_control_switches.return_switch << (shift * 1));
				msg.buttons |= (manual_control_switches.loiter_switch << (shift * 3));
				msg.buttons |= (manual_control_switches.offboard_switch << (shift * 5));
				msg.buttons |= (manual_control_switches.kill_switch << (shift * 6));
			}

			if (PX4_ISFINITE(manual_control_setpoint.aux1)) {
				msg.enabled_extensions |= (1u << 2);
				msg.aux1 = manual_control_setpoint.aux1 * 1000.f;
			}

			if (PX4_ISFINITE(manual_control_setpoint.aux2)) {
				msg.enabled_extensions |= (1u << 3);
				msg.aux2 = manual_control_setpoint.aux2 * 1000.f;
			}

			if (PX4_ISFINITE(manual_control_setpoint.aux3)) {
				msg.enabled_extensions |= (1u << 4);
				msg.aux3 = manual_control_setpoint.aux3 * 1000.f;
			}

			if (PX4_ISFINITE(manual_control_setpoint.aux4)) {
				msg.enabled_extensions |= (1u << 5);
				msg.aux4 = manual_control_setpoint.aux4 * 1000.f;
			}

			if (PX4_ISFINITE(manual_control_setpoint.aux5)) {
				msg.enabled_extensions |= (1u << 6);
				msg.aux5 = manual_control_setpoint.aux5 * 1000.f;
			}

			if (PX4_ISFINITE(manual_control_setpoint.aux6)) {
				msg.enabled_extensions |= (1u << 7);
				msg.aux6 = manual_control_setpoint.aux6 * 1000.f;
			}

			mavlink_msg_manual_control_send_struct(_mavlink->get_channel(), &msg);

			return true;
		}

		return false;
	}
};

#endif // MANUAL_CONTROL
