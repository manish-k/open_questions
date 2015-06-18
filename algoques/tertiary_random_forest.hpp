#pragma once 

#include "utils.hpp"

namespace AlgoComp {

  class InputChangeListener {
  public:
    virtual ~InputChangeListener ( ) { };

    virtual double OnInputChange ( unsigned int input_index_, double input_value_ ) = 0;
  };


  class OutputChangeListener {
  public:
    virtual ~OutputChangeListener ( ) { };

    virtual void OnOutputChange ( double _new_out_value_ ) = 0;
  };


  class TertiaryRandomForest : public InputChangeListener {
  public:
    TertiaryRandomForest ( ) { }
    
    //Will be called for initializing the Forest object
    //Can use Forest Class in utils.hpp to parse the forestfile
    void InitializeForest ( const char * const forest_filename_ ) { } /// needs to be implemented
    
    //Will be called to notfiy changes in predictor values
    //Should return the updated output value
    double OnInputChange ( unsigned int input_index_, double input_value_ ) { return 0; } /// needs to be implemented
    
    //_new_listener_ shoud be notified by calling _new_listener_->OnOutputChange on every change in Forest Output
    //should return true if _new_listener_ is successfully subscribed to Forest output updates
    bool SubscribeOutputChange ( OutputChangeListener * _new_listener_ ) { } ///needs to be implemented

  };

}
