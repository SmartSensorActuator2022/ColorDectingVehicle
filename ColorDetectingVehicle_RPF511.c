#pragma config(Sensor, S1, ts , sensorEV3_Touch)
#pragma config(Sensor, S2, gs , , sensorEV3_Gyro)
#pragma config(Sensor, S3, cs, sensorEV3_Color, modeEV3Color_Color)
#pragma config(Sensor, S4, ss, sensorEV3_Ultrasonic)
#pragma config(Motor, motorB, lm, tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor, motorC, rm, tmotorEV3_Large, PIDControl, encoder)

#define BLACK 1
#define BLUE 2
#define GREEN 3
#define YELLOW 4
#define RED 5
#define WHITE 6
#define BROWN 7
#define STARTCOLOR 4

#define FORWARD 0
#define ONCOLOR 1
#define TURNING 2
#define SLOWDOWN 3
#define OBSTACLE 4
#define BEEPBEEP 5
#define STARTED 6

#define TICKRATE 20
#define COLOR_QUEUE_SIZE 20

#define SPEED_MAX 8

char * num_to_color(int col);
void print_stat(void);
void init_color_queue(void);
int wait_touch(void);
void update_color(void);
int calculate_current_color(void);
void setstat(int s);
void addstat(int shiftleft);
void delstat(int shiftleft);
int checkstat(int shiftleft);
void setMotor(int leftspeed, int rightspeed);
void beepbeep(int count);
void point_turn_by_angle(int angle_ex, int degree);
void proceed_by_s(int sp, int s);
void distance_decision(void);
void color_decision(void);
void action(void);

// 0000000 : stop
// 0000001 : forward
// 0000010 : in some color
// 0000100 : turning
// 0001000 : slow down
// 0010000 : on obstacle
// 0100000 : beepbeep
// 1000000 : start
int stat = 0;


int color_queue[COLOR_QUEUE_SIZE];
int color_num[8];
int color_queue_index = 0;
int ex_color = 6;
int current_color = 6;

int black_count = 0;

//int beepbeep_count = 0;
//int beepbeep_cooldown = 0;

float time = 0;

int speed = SPEED_MAX;
int escape_dir = 1;
int distancein = 0;

task main()
{
	//clearSounds();
	resetGyro(gs);
	while(1){

		update_color();
		print_stat();

		color_decision();
		print_stat();
		if(!checkstat(TURNING)){
			distance_decision();
		}
		action();
		print_stat();
	}
}
char * num_to_color(int col){
	if(col == BLACK) return "BLACK";
	if(col == BLUE) return "BLUE";
	if(col == GREEN) return "GREEN";
	if(col == YELLOW) return "YELLOW";
	if(col == RED) return "RED";
	if(col == WHITE) return "WHITE";
	if(col == BROWN) return "BROWN";
	return "NOTCOLOR";
}

void print_stat(){
	displayBigTextLine(3, "%s %s %s %s %s %s %s",(checkstat(FORWARD)?"F":"-"),(checkstat(ONCOLOR)?"C":"-"),(checkstat(TURNING)?"T":"-"),(checkstat(SLOWDOWN)?"S":"-"),(checkstat(OBSTACLE)?"O":"-"),(checkstat(BEEPBEEP)?"B":"-"),(checkstat(STARTED)?"S":"-"));
	displayBigTextLine(5, "e %s c %s",num_to_color(ex_color),num_to_color(current_color));
	displayBigTextLine(7, "bc %d",black_count);
	displayBigTextLine(9, "gyro %d",getGyroDegrees(gs));
}


void init_color_queue(){
	for(int i = 0 ; i < COLOR_QUEUE_SIZE ; i++ ){
		color_queue[i] = 6;

	}
	for(int i = 0 ; i < 8 ; i++ ){
		color_num[i] = 0;
	}
	color_num[6] = COLOR_QUEUE_SIZE;
	color_queue_index = 0;
	current_color = 6;
	black_count = 0;
	//beepbeep_count = 0;
}

int wait_touch(void){
	while(getTouchValue(ts) == 0){}
	while(getTouchValue(ts) == 1){}
	return 1;
}

void update_color(void){
	int col = getColorName(cs);

	//for green
	/*
	if(col == 3){
		displayBigTextLine(15, "green");
		updatestat(3);
		current_color = GREEN;
		return;
	}*/

	if(col > 7 || col == 0) return;

	color_num[color_queue[color_queue_index]]--;
	color_num[col]++;

	if( color_queue[color_queue_index] != col){
		color_queue[color_queue_index] = col;
		current_color = calculate_current_color();
	}
	color_queue_index = (color_queue_index + 1) % COLOR_QUEUE_SIZE;
	return;
}

int calculate_current_color(void) {
	int max = 0;
	int temp_color = 6;
	for(int i = 1; i < 8; i++){
		if(color_num[i] > max){
			max = color_num[i];
			temp_color = i;
		}
	}
	return temp_color;
}

void setstat(int s){
	stat = s;
	return;
}

void addstat(int shiftleft){
	int base = 1 << shiftleft;
	stat = stat | base;
	return;
}

void delstat(int shiftleft){
	int base = ~(1 << shiftleft);
	stat = stat & base;
	return;

}

int checkstat(int shiftleft){
	int base = (1 << shiftleft);
	return stat & base;
}


void setMotor(int leftspeed, int rightspeed){
	setMotorSpeed(lm, leftspeed);
	setMotorSpeed(rm, rightspeed);

}
void beepbeep(int count) {
	for(int i = 0 ; i< count ; i++){
		displayBigTextLine(11, "beep loop %d to %d",i,count);
		playSound(soundBeepBeep);

	}
	delstat(BEEPBEEP);
	//clearSounds();
	displayBigTextLine(11, "beep loop done");

	/*
	int cooldown = 0;
	while (count > 0) {
		if(cooldown){
			cooldown = 0;
			count--;
			sleep(5);
		}
		else{
			cooldown = 1;
			playSound(soundBeepBeep);
			sleep(2);
		}
	}*/
}

void point_turn_by_angle(int angle_ex, int degree){
	if(degree > 0 ) {
		/*
		repeatUntil(getGyroDegrees(gs) < angle_ex + degree){
			setMotor(-speed,speed);
		}*/
		setMotor(speed,-speed);

		while(getGyroDegrees(gs) < angle_ex + degree){
			displayBigTextLine(9, "g %d to %d ",getGyroDegrees(gs),angle_ex + degree);
			sleep(TICKRATE);
		}
		return;
	}else{
	/*
		repeatUntil(getGyroDegrees(gs) < angle_ex + degree){
			setMotor(speed,-speed);
		}*/
		setMotor(-speed,speed);

		while(getGyroDegrees(gs) > angle_ex + degree){
			displayBigTextLine(9, "g %d to %d",getGyroDegrees(gs),angle_ex + degree);
			sleep(TICKRATE);
		}
		return;
	}
}
void proceed_by_s(int sp, int s){
	setMotor(sp,sp);
	sleep(1000*s / abs(sp));
}

void distance_decision(void){
	if(getUSDistance(ss) > 20 && !checkstat(TURNING)){
		delstat(SLOWDOWN);
		distancein = 0;
	} else if (!checkstat(TURNING) && checkstat(FORWARD)){
		if(getUSDistance(ss) < 20){
			if(!distancein){
				beepbeep(1);
				distancein = 1;
			}
			addstat(SLOWDOWN);
		}
		if(getUSDistance(ss) < 10){
				distancein = 1;
				addstat(SLOWDOWN);
				addstat(TURNING);
				delstat(ONCOLOR);
				escape_dir = 2;
				return;
		}

	}


}

void color_decision(void){
	if(current_color != WHITE && current_color != 0){
		ex_color = current_color;
		addstat(ONCOLOR);
		return;
	}


	if(checkstat(ONCOLOR) && current_color==WHITE){
		if(ex_color == BLACK){
			//if(++black_count == 10) delstat(FORWARD);
			black_count++;
		} else if(ex_color == BLUE){
			addstat(BEEPBEEP);
		 	//speed = 0;
		} else if(ex_color == STARTCOLOR){
			if(!checkstat(STARTED)){
				addstat(ONCOLOR);
				addstat(STARTED);
				addstat(FORWARD);
				clearTimer(T1);
				setLEDColor(ledGreen);
			} else{
				delstat(FORWARD);
			}
		}
		if(ex_color == RED){
				addstat(ONCOLOR);
				addstat(SLOWDOWN);
				addstat(TURNING);
				escape_dir = 1;
		}
		ex_color = WHITE;
		delstat(ONCOLOR);
		return;
	}


	/*
	if(!checkstat(ONCOLOR)){
		if(current_color == BLACK){
			addstat(ONCOLOR);
			if(++black_count == 10) delstat(FORWARD);
			return;
		}
		if(current_color == BLUE){
			addstat(ONCOLOR);
			addstat(BEEPBEEP);
		 	speed = 0;
		 	return;
		}
		if(current_color == RED){
			addstat(ONCOLOR);
			addstat(SLOWDOWN);
			addstat(TURNING);
			escape_dir = 1;
			return;
		}
		if(current_color == STARTCOLOR){
			if(!checkstat(STARTED)){
				addstat(ONCOLOR);
				addstat(STARTED);
				addstat(FORWARD);
				clearTimer(T1);
				setLEDColor(ledGreen);
				return;
			} else{
				delstat(FORWARD);
				return;
			}
		}
	}	else{
		if(current_color == WHITE) delstat(ONCOLOR);
	}*/
}

void action(void){
	print_stat();
	if(checkstat(SLOWDOWN)){
		speed = SPEED_MAX/2;
	}else{
		speed = SPEED_MAX;
	}

	if(checkstat(TURNING)){
		if(escape_dir == 1){
			point_turn_by_angle(getGyroDegrees(gs), -90);
			proceed_by_s(speed, 10*black_count);
			point_turn_by_angle(getGyroDegrees(gs), -90);
			delstat(TURNING);
			delstat(SLOWDOWN);
			delstat(ONCOLOR);
		}
		if(escape_dir == 2){
			proceed_by_s(-speed, 20);
			point_turn_by_angle(getGyroDegrees(gs), 90);
			proceed_by_s(speed, 10*black_count);
			point_turn_by_angle(getGyroDegrees(gs), 90);
			delstat(TURNING);
			delstat(SLOWDOWN);
		}

	}

	if(checkstat(BEEPBEEP)){
		//setMotor(0,0);
		if(black_count==0) beepbeep(1);
		else beepbeep(black_count);

		speed = SPEED_MAX;

	}
	if(black_count == 10) delstat(FORWARD);

	if(!checkstat(FORWARD)){
		setMotor(0,0);
		if(checkstat(STARTED)){
			time = time1[T1];
			displayBigTextLine (1, "record = %.2f sec", time);
			delstat(STARTED);
		}else{
			init_color_queue();
			black_count = 0;
			if(wait_touch()){
				addstat(FORWARD);
			}
		}
	}

	if(checkstat(FORWARD)&& !checkstat(TURNING)){
		setMotor(speed,speed);

	}
	sleep(TICKRATE);

}
