#include "utils.hpp"
#include "tertiary_random_forest.hpp"

int main ( int argc, char ** argv )
{
  if ( argc < 3 )
    {
      std::cerr << "Usage:: forestfile inputdatafile outputdatafile [samplingrate=1000]" << std::endl;
      exit ( 0 );
    }

  std::string forest_filename_ = std::string ( argv[1] );
  std::string inputdatafilename_ = std::string ( argv[2] );
  std::string outputdatafilename_ = std::string ( argv[3] );
  unsigned int samplingrate_ = 1000u;
  if ( argc > 4 ) { 
    samplingrate_ = std::max ( 1, atoi ( argv[4] ) ) ;
  }
  
  std::ifstream inputdatafile_;
  inputdatafile_.open ( inputdatafilename_.c_str(), std::ifstream::in ) ;

  std::ofstream outputdatafile_;
  outputdatafile_.open ( outputdatafilename_.c_str(), std::ios::out ) ;

  const int kDataLineBufferLen = 1024 ;
  char readline_buffer_ [ kDataLineBufferLen ];
  bzero ( readline_buffer_, kDataLineBufferLen );
  
  AlgoComp::cyclecount_t total_time_taken_ = 0;
 
  AlgoComp::TertiaryRandomForest tertiary_random_forest_;
  tertiary_random_forest_.InitializeForest ( forest_filename_.c_str() );

  std::vector < double > last_indicator_values_ ;

  const unsigned int buf_capacity_ = 1024000;
  char in_memory_buffer_ [ buf_capacity_ ] ; ///< memory storage for the text to be output. 
  unsigned int front_marker_ = 0;

  unsigned int samplingcounter_ = 0;
  while ( inputdatafile_.good ( ) )
    {
      bzero ( readline_buffer_, kDataLineBufferLen );
      inputdatafile_.getline ( readline_buffer_, kDataLineBufferLen ) ;
      AlgoComp::PerishableStringTokenizer st_ ( readline_buffer_, kDataLineBufferLen );
      const std::vector < const char * > & tokens_ = st_.GetTokens ( );
      if ( tokens_.size ( ) < 2 )
	continue;
      
      double base_price_ = atof ( tokens_[1] ); // the real target price is sum of base_price and forest_delta_value_
      if ( last_indicator_values_.empty ( ) )
	{
	  for ( unsigned int i = 2u; i < tokens_.size (); i ++ )
	    {
	      last_indicator_values_.push_back ( atof ( tokens_[i] ) ) ;
	    }
	}
      else
	{
	  std::cerr << "Time " << tokens_[0] << "\n";
	  for ( unsigned int i = 2u; i < tokens_.size (); i ++ )
	    {
	      last_indicator_values_[i-2u] = atof ( tokens_[i] ) ;
	      // As of now, calling OnInputChange for every variable change
	      // Later we can change this to only significant changes only
	      // Perhaps calculation of what is a significant change is best done inside the TertiaryRandomForest ?
	      AlgoComp::cyclecount_t prev_call_ = AlgoComp::GetCpucycleCount();
	      double forest_delta_value_ = tertiary_random_forest_.OnInputChange ( i-2u, last_indicator_values_[i-2u] ) ;
	      AlgoComp::cyclecount_t after_call_ = AlgoComp::GetCpucycleCount();
	      total_time_taken_ += ( after_call_ - prev_call_ );

	      samplingcounter_ ++;
	      if ( samplingcounter_ >= samplingrate_ )
		{
		  // print value for checking
		  int gcount = ::snprintf ( in_memory_buffer_ + front_marker_, ( buf_capacity_ - front_marker_ - 1u ), "%s %d %f\n", tokens_[0], i-1u, forest_delta_value_ ) ;
		  front_marker_ += gcount;
		  samplingcounter_ = 0;
		}

	    }
	}

      outputdatafile_.write ( in_memory_buffer_, front_marker_ ) ;
      front_marker_ = 0;
    }  
  outputdatafile_.close();
  
  std::cout << "Outputfile: " << outputdatafilename_ << " written. " << std::endl;
  std::cout << "Computation cycles: " << total_time_taken_ << std::endl;
}
