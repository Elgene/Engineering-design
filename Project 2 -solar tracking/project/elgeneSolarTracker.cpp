#include <iostream>
# include <time.h>
# include "E101.h"	//Import libraries
/*
 * Quick reference for copy + paste:
 * 		Host:	greta-pt.ecs.vuw.ac.nz
 * 		Port: 	22
 * */

//declare methods
	//method #1 isPixelRed()
		//return true false OR 0,1
	//method #2 moveHorizontal()
	//method #3 moveVerticle()
	//method #4 getRedCentreRow()
		//return int
	//getRedCentreCol()
		//return int
		
	//Program needs a 2D 240 rows * 320 colomns
		//See slide 7/19 for image processing part 1 in lecture schedule

class Tracker{
	private:
	//declare fields and variables here
	const int width = 320;
	const int height = 240;
	const int middleCol = width/2;
	const int middleRow = height/2;
	const int motorX = 1;
	const int motorY = 5;
	
	
	public:
	Tracker(bool debug1){
		debug = debug1;
		redRfirst = 0;
		redRlast = 0;
		redCentreCol = Tracker::middleCol;
		redCentreRow = Tracker::middleRow;
		redCfirst = 0;
		redClast = 0;
		currX = 48;
		currY = 48;
		totalRed = 0;
		lastMaxCol = 0;
		lastMaxRow = 0;
		
		
	}//constructor
	//declare methods here
	int currX;	//fields to store the current x & y position of the motors
	int currY;
	int redRfirst;	//fields for core get red row / col
	int redRlast;
	int redCfirst;
	int lastMaxRow;
	int lastMaxCol;
	int redClast;
	int totalRed;
	int redCentreCol;	//Stores the colomn that contains the middle of the circle
	int redCentreRow;	//Stores the row that contains the middle of the circle
	bool debug; //Add paramter to your methods that use this as an argument if you want your methods to have special behaviour for debugging
	bool isPixelRed(int row, int col, bool debug);
	void getImage(bool debug);
	int initialise();
	void motorTestX(int x);
	void motorTestY(int y);
	void motorReset();
	void moveMotor(int mode);
	void augmentedTest();
	void getRedCentre(int mode);	//mode 0 = core \\ mode 1 = completion \\ mode 2 = challenge
	void getReds(int mode); //mode 0 for ROW \\ mode 1 for COL
	void drawMiddle(); 
	bool noSun();
	};
//Write the methods here, using this format: returnType ClassName::methodName(parameters){code...};
/**
 *Return true if pixel at row,col is red 
 * **/
bool Tracker::isPixelRed(int row, int col, bool debug){ /*tested & working 9:30pm 09/08/2019 DD-MM-YY*/
	//checking using the get_pixel method and inputing in the from the int row and col 
	double red 			= get_pixel(row,col,0);
	double green 		= get_pixel(row,col,1);
	double blue 		= get_pixel(row, col, 2);
	double luminosity 	= get_pixel(row,col,3);
	double redBlueRatio = (double)red/(double)blue;
	double redGreenRatio = (double)red/(double)green;
	double redLumRatio = red / luminosity;
	if(debug){printf(" red blue ratio: %f \n red green ratio: %f \n red luminosity ratio: %f \n",redBlueRatio,redGreenRatio,redLumRatio);};
	//check for red, else not red == make the ratios smaller to increase sensitivity (i.e. be more loose when deciding if pixel is red or not)
	if(redBlueRatio > 2.2 && redGreenRatio > 2.2 && redLumRatio > 1.5){ 
		if(debug){printf("Red\n");}
		return true;
	}else{
		if(debug){printf("Not Red\n");}
		return false;
	}
};

/**
 *Makes !red pixels white on the screen
 *Makes red pixels red on the screen
 *used for testing the colour checking method
 * */
void Tracker::augmentedTest(){	/*Tested & Working 09-aug*/
	//printf("loops are running");
	for(int row = 0; row < Tracker::height; row++){
		//printf("Row: %d \n",row);
		for(int col = 0; col < Tracker::width; col++){
			if(isPixelRed(row,col,Tracker::debug)){
					set_pixel(row,col,255,1,1);
			}else{
					set_pixel(row,col,255,255,255);
			}
		}
	}
};

/**
 * Initialises the raspberry pi to take images and display them to the screen
 * */
int Tracker::initialise(){	/*Tested & working 07-08-2019 dd-mm-yy*/
		init(0);
		open_screen_stream();
		return 0;
};

/**
 * Takes an image, pushes it to the display for viewing, also makes image array availble for other methods
 * */
void Tracker::getImage(bool debug){	/*Tested & working 07-08-2019 dd-mm-yy*/
	take_picture();
	//update_screen();	
	if(debug){	//take 25 pictures
		for(int i = 0; i < 24; i++){
			take_picture();
			//update_screen();
			//augmentedTest();
			sleep1(50);
		}
	}
};

/**
 * Draws a horizontal and verticle line through the cerntre of the circle
 * */
void Tracker::drawMiddle(){	/*Tested & working 11-Aug*/
	for(int row = 0; row < 240; row++){
			set_pixel(row,Tracker::redCentreCol,10,255,0);
		}
		for(int col = 0; col < 320; col++){
			set_pixel(Tracker::redCentreRow,col,10,255,0);
		}
};

/**
 * Scans array for red mixels
 * If there aren't enough red pixels return false
 * */
bool Tracker::noSun(){
	int threshold = 1100;
	int red = Tracker::totalRed;
	if(red > threshold){ //The sun is detected
		Tracker::totalRed = 0;
		return true;
	}else{
		Tracker::totalRed = 0;
		return false;
	}
	
	
};

/**
 * Updates the Tracker fields:
 * redCentreRow & redCentreCol to the middle position of the circle
 *
 * depending on the mode, a different technique is used
 * */
void Tracker::getRedCentre(int mode){ /*Mode 1 needs testing, mode 2 need to be made*/
	//completion style (mode 2) --> center row / col has the maximum number of red pixels
	Tracker::totalRed = 0;
	if(mode < 0 || mode > 2){//validate mode input
		if(Tracker::debug){printf("please select mode 0 - 2");};
		return;
	}
	if(mode == 0){	//using core technique
		Tracker::getReds(0);
		Tracker::getReds(1);
		int rowMin = Tracker::redRfirst;			//first row to be red
		int rowMax = Tracker::redRlast;				//last row to be red
		
		Tracker::redCentreRow =  ((rowMin+rowMax) / 2);
		int colMin = Tracker::redCfirst;
		int colMax = Tracker::redClast;
		
		Tracker::redCentreCol = ((colMin + colMax)/2);
		
	}
	else if(mode == 1){	//using completion technique
		//Scan through rows, count how many colomns in that row have red in them, store as a total
		//update field to row with most red
		int maxRow = 0;
		int lastMax = 0; //amount of red in last maxRow
		for(int row = 0; row < 240; row++){
			int redColomns = 0;
				for(int col = 0; col < 320; col++){
					if(isPixelRed(row,col,Tracker::debug)){
						redColomns++;
						Tracker::totalRed++;
					}
			}
			if(redColomns > lastMax){
				lastMax = redColomns;
				maxRow = row;
			}
		}
		Tracker::redCentreRow = maxRow;
		int maxCol = 0;
		int lastMaxC = 0; //amount of red in last maxcolomn
		for(int col = 0; col < 320; col++){
			int redRows = 0;
				for(int row = 0; row < 240; row++){
					if(isPixelRed(row,col,Tracker::debug)){
						redRows++;
					}
			}
			if(redRows > lastMaxC){
				lastMaxC = redRows;
				maxCol = col;
			}
		}
		Tracker::redCentreCol = maxCol;
		Tracker::lastMaxCol = lastMaxC;
		Tracker::lastMaxRow = lastMax;
		return;
	}
	else if(mode == 2){	//using challenge technique
		return;
	}
	
	return;
};

/**
 * Helper method for getCentre method mode 1
 * Use mode = 0 for row
 * Use mode = 1 for colomn
 * */
 void Tracker::getReds(int mode){
	 if(mode == 0){ //find 1st red row & store in field + store last red Row in field
		 int firstRedRow = 0;
		 bool firstFound = false;
		 int lastRedRow = 0;
		 for(int row = 0; row < Tracker::height; row++){
			 for(int col = 0; col < Tracker::width; col++){
				 
				 if(isPixelRed(row,col,Tracker::debug)){	 
					 if(!(firstFound)){//if the pixel is red and none before have been red
											if(Tracker::debug){	//draw line
											for(int c = 0; c < Tracker::width; c++){set_pixel(row,c,0,255,0);} // draws horizontal green line
											}
											firstRedRow = row;
											firstFound = true;
									  }
			 
					lastRedRow = row;//if the pixel is red, set this row as the last row to have red pixel
				}
		 }//loops end here
		Tracker::redRfirst = firstRedRow;
		Tracker::redRlast = lastRedRow;
	 }
	 }
	 else if(mode == 1){	//find first red col + last red col
	 
		 int firstRedCol = 0;
		 int lastRedCol = 0;
		 bool firstFound = false;
		 for(int col = 0; col < Tracker::width; col++){
			 for(int row = 0; row < Tracker::height; row++){
				 
					if(isPixelRed(row,col,Tracker::debug)){
						if(!(firstFound)){//if the pixel is red and none before have been red
							if(Tracker::debug){	//draw line
								for(int r = 0; r < Tracker::height; r++){set_pixel(r,col,0,255,0);} // draws verticle green line
							}
								firstRedCol =  col;
								firstFound = true;
						}
						lastRedCol = col;//if the pixel is red, set this col as the last col to have red pixel
					}
			 }
		 }//loops end here
		 Tracker::redCfirst = firstRedCol;
		 Tracker::redClast = lastRedCol;
	 }
	 else
	 {
		 return;
	 }
	 
};

/**
 * method to test motors
 * 48 = middle/centre posiion
 * 32 = max right/up position
 * 65 = max left/down position
 * */
void Tracker::motorTestX(int x){
	int motorNumberx =1;	
	set_motors(motorNumberx,x);
	hardware_exchange();
	sleep1(50);
};

/**
 * method to test motors
 * 48 = middle/centre posiion
 * 32 = max right/up position
 * 65 = max left/down position
 * */
void Tracker::motorTestY(int y){	
	int motorNumbery =5; 
	set_motors(motorNumbery,y);
	hardware_exchange();
	sleep1(1);
	
};

/**
 * Sets Motors to default position
 * */
void Tracker::motorReset(){
	set_motors(Tracker::motorX,48);
	set_motors(Tracker::motorY,48);
	//update the motor
	Tracker::currX = 48;
	Tracker::currY = 48;
	hardware_exchange();
	sleep1(50);
};

/**
 * Move the camera to look at the sun
 * */
void Tracker::moveMotor(int mode){
	 bool row = true;
	 bool col = true;
	 Tracker::getRedCentre(mode); //change parameter for core, completion, challenge
	 
	 while(true){
		/*Check exit conditions*/
		printf("Total red: %d \n",Tracker::totalRed);
		if(!(Tracker::noSun())){
			printf("No sun detected\n");
			row = false;
			col = false;
			Tracker::motorReset();
			sleep1(500);
		}
		
		printf("max colomn red: %d row/2: %d \n",Tracker::lastMaxCol,Tracker::lastMaxRow/2);
		//printf("max row red: %d col/2: %d \n",Tracker::lastMaxRow,Tracker::lastMaxCol/2);
		if(!(Tracker::lastMaxCol > (Tracker::lastMaxRow/2)+10)){
			printf("Sun has left the domain\n");
			row = false;
			col = false;
			Tracker::motorReset();
			sleep1(500);
		}
		/*if(!(Tracker::lastMaxRow > Tracker::lastMaxCol/2)){
			printf("Sun has left the domain, second statement\n");
			row = false;
			col = false;
			Tracker::motorReset();
			sleep1(500);
		}*/
		
		/*These statements 'step' the camera position by 1 in the direction of the 'sun'
		 * & stop stepping the row & col is = sun position*/
		
		if(row){ 
			if(Tracker::redCentreRow > Tracker::middleRow){			//move y up
				Tracker::currY++;
			}else if(Tracker::redCentreRow < Tracker::middleRow){		//move y down
				Tracker::currY--;
			}
		}if(Tracker::redCentreRow > 100 && Tracker::redCentreRow < 150){//if the camera has moved its centre point close enough to the camera, stop stepping 
				row = false;
		}else{
				row = true;
		}
			
		if(col){
			if(Tracker::redCentreCol > Tracker::middleCol){	//move x right
				Tracker::currX++;
			}else if(Tracker::redCentreCol < Tracker::middleCol){	//move x left
				Tracker::currX--;
			}
		}if(Tracker::redCentreCol < 200 && Tracker::redCentreCol > 160){//if the camera has moved its centre point close enough to the camera, stop stepping 
				col = false;
		}else{
				col = true;
		}
		//change the motors
		set_motors(Tracker::motorX,Tracker::currX);
		set_motors(Tracker::motorY,Tracker::currY);
		hardware_exchange();
		sleep1(50);
		
		Tracker::getImage(Tracker::debug);//update the image array by taking a new image
		Tracker::augmentedTest();
		Tracker::drawMiddle();
		//printf("redPosition: row: %d Col: %d \n",Tracker::redCentreRow,Tracker::redCentreCol); //print the centre pos & camera position & difference between cam pos & middle POs
		update_screen();
		Tracker::getRedCentre(mode); //update the fields for the next iteration of the loop
		
		if(!(row && col)){
			
		}
		
			
	 }

};



int main()
{
	bool debug = false; //Toggle this to change program behaviour
	/*Remember to use "objectName.fieldName" to access fields*/
	Tracker track(debug);
	track.initialise();
	track.getImage(track.debug);
	track.moveMotor(1);
	track.motorReset();
	return 0;
}
