#include <iostream>
#include "E101.h"

using namespace std;

int main()
{
  int err;
  cout<<" Hello"<<endl;
  err = init(0);
  cout<<"After init() error="<<err<<endl;
  
 // int move_col= 0; 
//  int move_row=0;
  open_screen_stream();
  while(true)
  {
	 
	int max_col=0;
	int max_col_index = 1;
	int max_row=0;
	int max_row_index = 1;
	 set_motors(5,48);
	 set_motors(3,48);
	 hardware_exchange();
	 take_picture();
	 int hist_col[320] = {0};
	 int hist_row[240] = {0};

	 for ( int row = 0; row<240; row++){
		for ( int col = 0; col<320; col++){
		   int r = get_pixel(row,col,0);
		   int g = get_pixel(row,col,1);
		   int b = get_pixel(row,col,2);
		   if ((r>90)&&(g<60)&&(b<60)){
			   set_pixel(row, col,0,0,0);
			   hist_col[col]++;   
			   hist_row[row]++; 
			}	   
		}	
	 }
	 
	  for ( int col = 0; col<320; col++){

		if(hist_col[col]> max_col){  //max col
			   max_col = hist_col[col];
			   max_col_index = col ;
		 }
	   }
	   
	 for ( int row = 0; row<240; row++){ 
		   if(hist_row[row]>max_row){  //max row
			 max_row = hist_row[row];
			 max_row_index = row ;
			 
			  }
		   }
	printf("Row = %f\n", max_row_index); 
	printf("Col = %f\n", max_col_index);  
		   
	// if(max_col_index)
	 update_screen();

	 //set_motors(5,54);
	 //set_motors(3,54);
	 //hardware_exchange();
	 //take_picture();
	 //update_screen();
    }
    //save_picture("file1");
    close_screen_stream();
    stoph();
	
	return 0;
}
