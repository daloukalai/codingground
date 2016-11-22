////////////////////////////////////////////////////////////////////////
//
// File:       watchdog.cpp

////////////////////////////////////////////////////////////////////////


#include "watchdog.h"


// TODO Linux needs porting



////////////////////////////////
// WatchdogBase - Constructor
////////////////////////////////
//

WatchdogBase::WatchdogBase( const char* szDescriptor )
 : hSemWatchdog( PTHREAD_COND_INITIALIZER )
 , fValid      ( 0 )
 , fShuttingDown( 0 )
 , threadMutex(PTHREAD_MUTEX_INITIALIZER)
{
   int rc ;

   SetTickerDelay( 5 * 60 * 1000 ) ; // Default is 5 minutes
   rc = pthread_cond_init(&hSemWatchdog, NULL); // Might need to be defined with a condition

   // FALSE => synchronization timer (A timer whose state remains
   // signaled until a thread completes a wait operation on the timer object.)

   if ( rc !=0 )
   {
      cout << "Thread Condition Intializing error" << endl ;
   }
   else
   {
	   int err;
	   err = pthread_create(&thread, NULL, &fnWatchdogThread, this);
      if ( err !=0 )
        cout <<"Create Watchdog thread failed with err = " << err << endl ;
      else
      {
         for ( int iRepeat = 20 ; iRepeat > 0 ; --iRepeat )
         {
            Sleep(500) ; // Give up timeslice, allow thread to be created
            if ( fValid )
               break ;
         }
         if ( !fValid )
            cout << "Watchdog thread failed to start after 10 seconds." << endl ;
      }
   }

}

////////////////////////////////
// WatchdogBase - Destructor
////////////////////////////////
//

WatchdogBase::~WatchdogBase( )
{
   int rc ;
   //if ( thread != NULL)
  // {
      fShuttingDown = 1 ; // Ready to kill thread

      if ( hSemWatchdog )
      {
          alarm(0);
          rc = pthread_cond_destroy(&hSemWatchdog);
    	 if(rc != 0)
         {
            cout << "~WatchdogBase: condition destroy failed" << endl ;
         }
      }
   //}
   StopTicker( );
}

////////////////////////////////
// fnWatchdogThread
////////////////////////////////
//
void * __cdecl WatchdogBase::fnWatchdogThread( void * pV )
{
   WatchdogBase * pWatchdog = (Watchdog*)pV;
   int         rc ;
   pWatchdog->fValid = 1 ; // Ready to rock and roll

   for ( ;; )
   {
      // Wait 'forever' for the timer to expire
	   pthread_mutex_lock(&pWatchdog->threadMutex);
	   rc = pthread_cond_wait(&pWatchdog->hSemWatchdog, &pWatchdog->threadMutex);
      pthread_mutex_unlock(&pWatchdog->threadMutex);

      if ( pWatchdog->fShuttingDown )
         return 0;

      // Only get here if user didn't run StopTicker() in time.
      //   "Arf! Arf!"

      if ( rc != 0 )
      {
        cout << "Infinite condition wait has failed!" << endl ;
      }

      // Do what the user wants
      pWatchdog->Expiry() ;
   }
//#endif
   return 0;
   // Can NEVER get here !
}


////////////////////////////////
//    StopTicker
////////////////////////////////
//

void WatchdogBase::StopTicker( void )
{
   int   rc;
   if ( hSemWatchdog )
   {
	 // reset the timer - TODO below
	   alarm(0);
	 //if(!pthread_cond_destroy(&hSemWatchdog))
     //if ( ! CancelWaitableTimer( hSemWatchdog ) )
     //{
       //rc = ::GetLastError() ;
       //cout << "StopTicker: CancelWaitableTimer failed, rc = " << rc << endl ;
     //}
   }
//#endif
}

void signalCond( void )
{
	pthread_cond_signal(&hSemWatchdog);
}
////////////////////////////////
//    StartTicker
////////////////////////////////
//

void WatchdogBase::StartTicker( unsigned long ulDelay )
{
	int rc = 0;
   if ( ulDelay )
      SetTickerDelay( ulDelay ) ;

   // Allow user to call StartTicker() several times in a row
   // without an intervening StopTicker()
   StopTicker( );
#if defined(_WIN32)
   if ( hSemWatchdog )
   {
	   signal( SIGALRM, signalCond );
	   alarm( 10 ); // or use setitimer()
   }
#endif
}

////////////////////////////////////////////////////////////////////////
//  Implementation of concrete Watchdog (subclass of WatchdogBase)
////////////////////////////////////////////////////////////////////////
//
////////////////////////////////
//    Expiry
////////////////////////////////
//

void  Watchdog::Expiry( void )
{
   // Call users function
   if ( pfnExpiry != NULL )
      pfnExpiry( pVarg );
}

void callMeOnExpiry(void *)
{
	cout << "Function to hit after the time expiry!" << endl;
}
int main()
{
	const char * fnDesc = "SleepFnDescription";
	 Watchdog wDog(fnDesc, callMeOnExpiry, 0);
	 wDog.StartTicker(10000);
	 cout << "Start ticker done" << endl;
	 Sleep(20000);
	 cout << "Sleep task done" << endl;
	 wDog.StopTicker();
	 cout << "Stop ticker done" << endl;
	 return 0;
}
// end of Watchdog.CPP
