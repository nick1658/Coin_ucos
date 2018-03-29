#ifndef MOTOR_H
#define MOTOR_H


#define MOTOR0(x) OUT0(x)
#define MOTOR1(x) OUT1(x)
#define MOTOR2(x) OUT2(x)
#define MOTOR3(x) OUT3(x)
#define MOTOR4(x) OUT4(x)
#define MOTOR5(x) OUT5(x)
#define MOTOR6(x) OUT6(x)
#define MOTOR7(x) OUT7(x)



#define MOTOR0_DIR(x) OUT4(x)
#define MOTOR1_DIR(x) OUT5(x)
#define MOTOR2_DIR(x) OUT6(x)
#define MOTOR3_DIR(x) OUT3(x)
#define MOTOR4_DIR(x) OUT4(x)
#define MOTOR5_DIR(x) OUT5(x)
#define MOTOR6_DIR(x) OUT6(x)
#define MOTOR7_DIR(x) OUT7(x)


typedef struct {
	int32_t motor_id;
}s_motor_def;


int start_motor (int32_t motor_id, int32_t motor_step, int32_t motor_speed);
int set_motor_dir (int32_t motor_id, int32_t motor_dir);

#endif


