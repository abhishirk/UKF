#include <iostream>
#include "tools.h"

using Eigen::VectorXd;
using Eigen::MatrixXd;
using std::vector;

Tools::Tools() {}

Tools::~Tools() {}

VectorXd Tools::CalculateRMSE(const vector<VectorXd> &estimations,
                              const vector<VectorXd> &ground_truth) {
  /**
  TODO:
    * Calculate the RMSE here.
  */
	VectorXd rmse(4);
  rmse << 0,0,0,0;

    // TODO: YOUR CODE HERE

  // check the validity of the following inputs:
  //  * the estimation vector size should not be zero
  if(estimations.size()==0){
      
  }else if(estimations.size()!=ground_truth.size()){
      
  }else{
      for(unsigned int i=0; i < estimations.size(); ++i){
            // ... your code here
        VectorXd e=estimations[i];
        VectorXd g=ground_truth[i];
        VectorXd diff=e-g;
        VectorXd diffSquare=diff.array()*diff.array();
        rmse=rmse+diffSquare;
      }
  }
  //  * the estimation vector size should equal ground truth vector size
  // ... your code here

  //accumulate squared residuals
  

  //calculate the mean
  // ... your code here
    rmse=rmse/estimations.size();
  //calculate the squared root
  // ... your code here
    rmse=rmse.array().sqrt();
  //return the result
  return rmse;
}