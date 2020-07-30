#include <iostream>
#include "E101.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

using namespace std;

class ROBOT
{
private:
	
	int v_left; 
	int v_right;
	int cam_tilt; 
	int dv;
	double line_error;
	int n_black;
	int NumleftTurn = 0;
	bool paperFlag = false;
	
	const int cam_width = 320;
	const int cam_height = 240;
	const int v_left_go = 52;
	const int v_right_go = 43;
	double kp = 0.05;
	int line_present = 1;
	bool hardTurn= false;
	
public:
	ROBOT(){};
	int initHardware();
	void setMotors();
	int MeasureLine();	
	int FollowLine();
	int OpenGate();
	int turnLeft();
	int turnRight();
	int redFlag();
	
};

int ROBOT::OpenGate()
{   char server_addr[] = "130.195.6.196";
	connect_to_server(server_addr, 1024);
	char message[]= "Please";
	send_to_server(message);
	char password[24];
	receive_from_server(password);
	send_to_server(password);
	v_left = 60;											
    v_right = 35;
    setMotors();
    sleep1(800);
	return 1;
}

int ROBOT::initHardware()
{
    int err;
    cout<<" Hello"<<endl;
    err = init(0);
    cout<<"After init() error="<<err<<endl;
     v_left = 48;
     v_right = 47;
	 cam_tilt = 40; 
	dv=0;
	setMotors();
    set_motors(3, cam_tilt);
	
	
	return err;
}

 void ROBOT::setMotors()
 {
     //v_left = 53;      // speed range from 49 to 65 | left wheel motor 5
    //v_right = 42;     // speed rande from 47 to 30 | right wheel motor 1
	 cout<<"v_left= "<<v_left<<"\t v_right= "<<v_right;
	// cout<<"cam_tilt= "<<cam_tilt<<endl;
	 set_motors(5,v_left);
	 set_motors(1,v_right);
	
     set_motors(3, cam_tilt);
	 hardware_exchange();
 }
 	


int ROBOT::MeasureLine()
{
	take_picture();
	update_screen();
	
	int row = 120;
	int col = 0;
	int max = 0;
	int min = 256;
	
	for( col = 0; col < cam_width; col++)
	{
		int wh = get_pixel(row, col, 3);
		if (wh > max){ max = wh;}
		if (wh < min){ min = wh;}
	}
	line_present= 1;
	
	
	
	
	double throuse_hold = 70;
	cout<<"Threshold: " << throuse_hold  <<endl;
	line_error = 0.0;
	n_black = 0;
	cout << "|";
	for( col = 0; col < cam_width; col++)
	{
		if( get_pixel(row, col, 3) > throuse_hold)
		{
			cout << " ";
	    }	
		if( get_pixel(row, col, 3) <= throuse_hold)
		{	
			cout << "X";
			line_error += (col - cam_width /2.0);
			n_black++;
			
		}
	} 
	cout << "|" << endl;
	cout << "n_black: "<<n_black<<" ";
	
	   if(n_black>300){
			if(NumleftTurn < 2){
				turnLeft();
			}
			else{
			
			turnRight();
			}NumleftTurn++;
	   } cout<<"left turn:"<<NumleftTurn<<endl;
		
	
		
	
	if(n_black < 10)
	{
		line_present= 0;
		return -1;
	}
	
	line_present= 1;
	cout << "line error: " << line_error<<endl;
	line_error = line_error/n_black;
	return 0;
	
}

 int ROBOT::FollowLine()
 {  MeasureLine();
	 if (hardTurn==true){
	 hardTurn= false; 
	 return 0; 
	
  }
	 cout <<"Line Present: " << line_present << endl;
	 if(line_present)
	 {
		 dv =(int)(line_error*kp);
		 v_left = v_left_go + dv;
		 v_right =v_right_go + dv;
		 cout<<"line_error= "<<line_error<<"dv= "<<dv;
		 setMotors();
	 }
	 else
	 {
		 //go back
		 cout<<"line is missing"<<endl;
		 v_left = 43;
		 v_right = 52;
		 
		 setMotors();
		 sleep1(100);
	 }
	 return 0;
 }
int ROBOT::redFlag()
{ 
	// count number of red
	int n_red=0;
	int row = 120;
	int col = 0;
	for( col = 0; col < cam_width; col++)
	{
		
		if( get_pixel(row, col, 0) <= 50)
		{	
			//cout << "n_red col: "<<col<<" ";
			
			n_red++;
		}
	}
	
	 
	if (n_red < 20 && paperFlag == false) 
	{
		paperFlag = true;
		set_motors(5,53);
		set_motors(1,42);
		setMotors();
		sleep1(1000);
	
	}
	return 0;
}	


int ROBOT::turnLeft()
{   
	v_left = 48;       // speed range from 49 to 65 | left wheel motor 5
    v_right = 35;  // speed rande from 47 to 30 | right wheel motor 1
    setMotors();
    sleep1(2000);
    return 0;
}
int ROBOT::turnRight()
{   
	v_left = 60;
    v_right = 47;
    setMotors();
     sleep1(2000);
    return 0;
}
int main()
{
	ROBOT robot;
	
	robot.initHardware();
	robot.OpenGate();
	int count = 0;
	open_screen_stream();

	while( count < 5000)
	{
		take_picture();
		update_screen();
		robot.FollowLine();
		//robot.turnLeft();
		//robot.turnRight();
		robot.redFlag();
		
		
			
		}
		count++;
		
	
	close_screen_stream();
	stoph();
	return 0;

}
 
	
	
	

