uint64 timestamp # time since system start (microseconds)

float32[2] velocity_ned # 2 Dimensional velocity setpoint in NED frame
bool backwards	        # Flag for backwards driving
float32 yaw		# Vehicle yaw setpoint in NED frame (Exclusive with velocity_ned, the rover will turn on the spot to reach the yaw setpoint)

# TOPICS differential_velocity_setpoint
