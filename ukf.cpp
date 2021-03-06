#include "ukf.h"
#include "Eigen/Dense"
#include <iostream>

using namespace std;
using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::vector;

/**
 * Initializes Unscented Kalman filter
 * This is scaffolding, do not modify
 */
UKF::UKF() {
  // if this is false, laser measurements will be ignored (except during init)
  use_laser_ = true;

  // if this is false, radar measurements will be ignored (except during init)
  use_radar_ = true;

  // initial state vector
  x_ = VectorXd(5);

  // initial covariance matrix
  P_ = MatrixXd(5, 5);

  // Process noise standard deviation longitudinal acceleration in m/s^2
  std_a_ = 30;

  // Process noise standard deviation yaw acceleration in rad/s^2
  std_yawdd_ = 30;
  
  //DO NOT MODIFY measurement noise values below these are provided by the sensor manufacturer.
  // Laser measurement noise standard deviation position1 in m
  std_laspx_ = 0.15;

  // Laser measurement noise standard deviation position2 in m
  std_laspy_ = 0.15;

  // Radar measurement noise standard deviation radius in m
  std_radr_ = 0.3;

  // Radar measurement noise standard deviation angle in rad
  std_radphi_ = 0.03;

  // Radar measurement noise standard deviation radius change in m/s
  std_radrd_ = 0.3;
  //DO NOT MODIFY measurement noise values above these are provided by the sensor manufacturer.
  
  /**
  TODO:

  Complete the initialization. See ukf.h for other member properties.

  Hint: one or more values initialized above might be wildly off...
  */
  is_initialized_=false;
  n_aug_=7;
  n_x_=5;
  lambda=3-n_aug_;

  P_ << 1, 0, 0, 0, 0,
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 0, 1, 0,
        0, 0, 0, 0, 1;


}

UKF::~UKF() {}

/**
 * @param {MeasurementPackage} meas_package The latest measurement data of
 * either radar or laser.
 */
void UKF::ProcessMeasurement(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Make sure you switch between lidar and radar
  measurements.
  */

  if (!is_initialized_) {
    /**
    TODO:
      * Initialize the state ekf_.x_ with the first measurement.
      * Create the covariance matrix.
      * Remember: you'll need to convert radar from polar to cartesian coordinates.
    */
    // first measurement
    cout << "Initializing EKF: " << endl;
    x_ << 1, 1, 1, 1,1;
    cout << "State Initialized " << endl;
    if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
      /**
      Convert radar from polar to cartesian coordinates and initialize state.
      */
      float rho=meas_package.raw_measurements_[0];
      float phi=meas_package.raw_measurements_[1];
      cout << "Radar Initialization " << endl;
      x_ << rho*(cos(phi)),rho*(sin(phi)), 1, 0,0;
      cout << "Radar Initialization done" << endl;
    }
    else if (meas_package.sensor_type_ == MeasurementPackage::LASER) {
      /**
      Initialize state.
      */
      cout << "Lidar Initialization " << endl;
      x_ << meas_package.raw_measurements_[0], meas_package.raw_measurements_[1], 1, 0,0;
      cout << "Lidar Initialization done" << endl;
    }
    time_us_ = meas_package.timestamp_;
    cout << "Initialized X State : "<< time_us_ << endl;
    cout << "x_ = " << x_ << endl;
    cout << "P_ = " << P_ << endl;
    // done initializing, no need to predict or update
    is_initialized_ = true;
    cout << "Total Initialization done" << endl;
    return;
  }

  float dt = (meas_package.timestamp_ - time_us_) / 1000000.0; //dt - expressed in seconds
  time_us_ = meas_package.timestamp_;

  Prediction(dt);

  if (meas_package.sensor_type_ == MeasurementPackage::RADAR) {
    cout << "Radar update called " <<meas_package.raw_measurements_<< endl;  
    cout << "Radar update ended " << endl;
  } else {
    // Laser updates
    cout << "Laser update called " <<meas_package.raw_measurements_<< endl;  
    cout << "Laser update ended " << endl;
  }

}

/**
 * Predicts sigma points, the state, and the state covariance matrix.
 * @param {double} delta_t the change in time (in seconds) between the last
 * measurement and this one.
 */
void UKF::Prediction(double delta_t) {
  std::cout << "Prediction started" << std::endl;
  std::cout << "delta_t = "<< delta_t << std::endl;

  /**
  TODO:
  Complete this function! Estimate the object's location. Modify the state
  vector, x_. Predict sigma points, the state, and the state covariance matrix.
  */
  //create augmented mean vector
  VectorXd x_aug = VectorXd(n_aug_);
  
  //create augmented state covariance
  MatrixXd P_aug = MatrixXd(n_aug_, n_aug_);

  //create sigma point matrix
  MatrixXd Xsig_aug = MatrixXd(n_aug_, 2 * n_aug_ + 1);

  //create augmented mean state
  for(int i=0;i<5;i++){
      x_aug[i]=x_[i];
  }

  P_aug.topLeftCorner(n_x_,n_x_)=P_;
  P_aug(5,5)=std_a_*std_a_;
  P_aug(6,6)=std_yawdd_*std_yawdd_;
  //create square root matrix
  MatrixXd A = P_aug.llt().matrixL();
  //create augmented sigma points
  VectorXd T = VectorXd(n_aug_);
  T.fill(1);
  MatrixXd T_t=T.transpose();
  MatrixXd transform=x_aug*T_t;
  MatrixXd var=sqrt(3)*A;
  MatrixXd plus=transform+var;
  MatrixXd minus=transform-var;
  Xsig_aug<<x_aug,plus,minus;
  std::cout << "Xsig_aug = " << std::endl << Xsig_aug << std::endl;


  MatrixXd Xsig_pred = MatrixXd(n_x_, 2 * n_aug_ + 1);

  for (int i = 0; i< 2*n_aug_+1; i++)
  {
    //extract values for better readability
    double p_x = Xsig_aug(0,i);
    double p_y = Xsig_aug(1,i);
    double v = Xsig_aug(2,i);
    double yaw = Xsig_aug(3,i);
    double yawd = Xsig_aug(4,i);
    double nu_a = Xsig_aug(5,i);
    double nu_yawdd = Xsig_aug(6,i);

    //predicted state values
    double px_p, py_p;

    //avoid division by zero
    if (fabs(yawd) > 0.001) {
        px_p = p_x + v/yawd * ( sin (yaw + yawd*delta_t) - sin(yaw));
        py_p = p_y + v/yawd * ( cos(yaw) - cos(yaw+yawd*delta_t) );
    }
    else {
        px_p = p_x + v*delta_t*cos(yaw);
        py_p = p_y + v*delta_t*sin(yaw);
    }

    double v_p = v;
    double yaw_p = yaw + yawd*delta_t;
    double yawd_p = yawd;

    //add noise
    px_p = px_p + 0.5*nu_a*delta_t*delta_t * cos(yaw);
    py_p = py_p + 0.5*nu_a*delta_t*delta_t * sin(yaw);
    v_p = v_p + nu_a*delta_t;

    yaw_p = yaw_p + 0.5*nu_yawdd*delta_t*delta_t;
    yawd_p = yawd_p + nu_yawdd*delta_t;

    //write predicted sigma point into right column
    Xsig_pred(0,i) = px_p;
    Xsig_pred(1,i) = py_p;
    Xsig_pred(2,i) = v_p;
    Xsig_pred(3,i) = yaw_p;
    Xsig_pred(4,i) = yawd_p;
  }

      std::cout << "Xsig_pred = " << std::endl << Xsig_pred << std::endl;
      // Setting weight vector
      weights = VectorXd(2*n_aug_+1);
      weights[0]=lambda/(lambda+n_aug_);
      for(int i=1;i<15;i++){
          weights[i]=0.5/(n_aug_+lambda);          
      }
      VectorXd x = VectorXd(n_x_);
      x.fill(0);
      for(int i=0;i<15;i++){
          x=x+(weights[i]*Xsig_pred.col(i));
        //   std::cout<< "x = "<<x<<std::endl;
      }
      //create covariance matrix for prediction
      MatrixXd P = MatrixXd(n_x_, n_x_);      
      P.fill(0.0);
      for(int i=0;i<15;i++){
          VectorXd x_diff = Xsig_pred.col(i) - x;
          while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
          while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;
          P = P + weights(i) * x_diff * x_diff.transpose() ;
      }
      x_=x;
      P_=P;

      std::cout << "Prediction ended" << std::endl;
 }


/**
 * Updates the state and the state covariance matrix using a laser measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateLidar(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Use lidar data to update the belief about the object's
  position. Modify the state vector, x_, and covariance, P_.

  You'll also need to calculate the lidar NIS.
  */
}

/**
 * Updates the state and the state covariance matrix using a radar measurement.
 * @param {MeasurementPackage} meas_package
 */
void UKF::UpdateRadar(MeasurementPackage meas_package) {
  /**
  TODO:

  Complete this function! Use radar data to update the belief about the object's
  position. Modify the state vector, x_, and covariance, P_.

  You'll also need to calculate the radar NIS.
  */
}
