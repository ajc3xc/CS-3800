#include "ioModule.h"
#include "process.h"
#include "processMgmt.h"

#include <chrono> // for sleep
#include <thread> // for sleep

int main(int argc, char *argv[]) {
  // single thread processor
  // it's either processing something or it's not
  bool processorAvailable = true;

  // vector of processes, processes will appear here when they are created by
  // the ProcessMgmt object (in other words, automatically at the appropriate
  // time)
  // FROM process.h
  list<Process> processList;

  //  this will orchestrate process creation in our system, it will add
  //  processes to processList when they are created and ready to be run/managed
  ProcessManagement processMgmt(processList);

  // this is where interrupts will appear when the ioModule detects that an IO
  // operation is complete
  list<IOInterrupt> interrupts;

  // this manages io operations and will raise interrupts to signal io
  // completion
  IOModule ioModule(interrupts);

  // Do not touch
  long time = 1;
  long sleepDuration = 50;
  string file;
  stringstream ss;
  enum stepActionEnum {
    noAct,
    admitNewProc,
    handleInterrupt,
    beginRun,
    continueRun,
    ioRequest,
    complete
  } stepAction;

  // Do not touch
  switch (argc) {
  case 1:
    file = "./procList.txt"; // default input file
    break;
  case 2:
    file = argv[1]; // file given from command line
    break;
  case 3:
    file = argv[1];  // file given
    ss.str(argv[2]); // sleep duration given
    ss >> sleepDuration;
    break;
  default:
    cerr << "incorrect number of command line arguments" << endl;
    cout << "usage: " << argv[0] << " [file] [sleepDuration]" << endl;
    return 1;
    break;
  }

  /////////////////////////////////////////////Read
  /// procList.txt///////////////////////////////////////////////////////////////////////
  processMgmt.readProcessFile(file);
  // cout<<file<<endl;

  time = 0;

  // boolean state variables
  bool begin_run = false;
  bool continue_run = false;
  bool all_processes_done = false;

  // ready list and blocked list
  list<Process> readyList;
  list<Process> blockedList;

  // keep running the loop until all processes have been added and have run to
  // completion
  while(!all_processes_done  /* TODO add something to keep going as long as there are processes that arent done! */ )
    {
    // Update our current time step
    ++time;

    // let new processes in if there are any
    // add new process
    processMgmt.activateProcesses(time);

    // update the status for any active IO requests
    ioModule.ioProcessing(time);

    // If the processor is tied up running a process, then continue running it
    // until it is done or blocks
    //    note: be sure to check for things that should happen as the process
    //    continues to run (io, completion...)
    // If the processor is free then you can choose the appropriate action to
    // take, the choices (in order of precedence) are:
    //  - admit a new process if one is ready (i.e., take a 'newArrival' process
    //  and put them in the 'ready' state)
    //  - address an interrupt if there are any pending (i.e., update the state
    //  of a blocked process whose IO operation is complete)
    //  - start processing a ready process if there are any ready

    // init the stepAction, update below

    stepAction = noAct;

    // TODO add in the code to take an appropriate action for this time step!
    // you should set the action variable based on what you do this time step.
    // you can just copy and paste the lines below and uncomment them, if you
    // want. stepAction = continueRun;  //runnning process is still running
    // stepAction = ioRequest;  //running process issued an io request
    // stepAction = complete;   //running process is finished
    // stepAction = admitNewProc;   //admit a new process into 'ready'
    // stepAction = handleInterrupt;   //handle an interrupt
    // stepAction = beginRun;   //start running a process

    //   <your code here>

    /*--------------------
      Processor is free
    ---------------------*/
    // admit new processes (1st in heirarchy if free processor)
    if (processorAvailable && !continue_run) {
      // admit in any new processes
      for (list<Process>::iterator processIt = processList.begin();
           processIt != processList.end(); processIt++) {
        if (processIt->state == newArrival) {
          processorAvailable = false;
          processIt->state = ready;
          readyList.push_back(*processIt);
          stepAction = admitNewProc;
          break;
        }
      }
    }

    // move process with finished IO interrupt to ready list
    // (2nd in heirarchy if free processor)
    if (processorAvailable) {
      for (list<Process>::iterator blockedIt = blockedList.begin();
           blockedIt != blockedList.end(); blockedIt++) {
        // cout << blockedIt->id << endl;
        if (blockedIt->id ==
            interrupts.front().procID) // TODO: move to ready when not io block
        {
          processorAvailable = false;
          unsigned int blockedProcessID = blockedIt->id;
          for (list<Process>::iterator processIt = processList.begin();
               processIt != processList.end(); processIt++) {
            // cout<<blockedIt->id<<endl;
            if (processIt->id == blockedProcessID) {
              processIt->state = ready;
              readyList.push_back(*processIt);
              blockedList.erase(blockedIt);
              interrupts.pop_front();
              break;
            }
          }
          stepAction = handleInterrupt;
          break;
        }
      }
    }

    // begin running a process (last in heirarchy if free processor)
    // effectively switches thing
    if (processorAvailable) {
      for (list<Process>::iterator readyIt = readyList.begin();
           readyIt != readyList.end(); readyIt++) {
        processorAvailable = false;
        unsigned int readyProcessID = readyIt->id;
        for (list<Process>::iterator processIt = processList.begin();
             processIt != processList.end(); processIt++) {
          if (processIt->id == readyProcessID) {
            processIt->state = processing;
          }
        }
        readyList.pop_front();
        begin_run = true;
        break;
      }
    }
    processorAvailable = true;
    /*--------------------
      Processor is free
    ---------------------*/

    // this if-else if runs the process
    if (continue_run) { // continuing to run from previous cycles
      stepAction = continueRun;
      processorAvailable = false;
      for (list<Process>::iterator processIt = processList.begin();
           processIt != processList.end(); processIt++) {
        if (processIt->state == processing) {
          processIt->processorTime++;
          for (list<IOEvent>::iterator ioIt = processIt->ioEvents.begin();
               ioIt != processIt->ioEvents.end(); ioIt++) {
            if (processIt->processorTime ==
                ioIt->time) // only works if there is one
                            // io event at a time (BUG)
            {
              processIt->state = blocked;
              stepAction = ioRequest;
              blockedList.push_back(*processIt);
              ioModule.submitIORequest(time, processIt->ioEvents.front(),
                                       *processIt);
              processorAvailable = true;
              continue_run = false;
            }
          }
          if (processorAvailable == false) // check if process got blocked
          {
            if (processIt->processorTime <
                processIt->reqProcessorTime) { // not done
              break;
            } else { // process finished running
              processIt->state = done;
              continue_run = false;
              processorAvailable = true;
              stepAction = complete;
            }
          }
        }
      }
    } else if (begin_run) { // just started running this cycle
      processorAvailable = false;
      stepAction = beginRun;
      begin_run = false;
      continue_run = true;
      /*
      for (list<Process>::iterator processIt = processList.begin();
           processIt != processList.end(); processIt++) {
        if (processIt->state == processing) {
          // processIt->processorTime++;

          for (list<IOEvent>::iterator ioIt = processIt->ioEvents.begin();
               ioIt != processIt->ioEvents.end(); ioIt++) {
            if (processIt->processorTime ==
                ioIt->time) // only works if there is one
                            // io event at a time (BUG)
            {
              processIt->processorTime++;
              processIt->state = blocked;
              stepAction = ioRequest;
              blockedList.push_back(*processIt);
              ioModule.submitIORequest(time, processIt->ioEvents.front(),
                                       *processIt);
              processorAvailable = true;
              continue_run = false;
            }

          }
          */
      /*if (processorAvailable == false) // check if processor got blocked
        if (processIt->processorTime <
            processIt->reqProcessorTime) { // not done
          break;
        } else { // process finished running
          processIt->state = done;
          continue_run = false;
          processorAvailable = true;
          stepAction = complete;
        }
    }
  }*/
    }

    // check if all processes are done running
    int processListSize = processList.size();
    int index = 0;
    for (list<Process>::iterator processIt = processList.begin();
         processIt != processList.end(); processIt++) {
      if (processIt->state == done) {
        if (index >= processListSize - 1)
          all_processes_done = true;
      } else
        break;
      index++;
    }

    // Leave the below alone (at least for final submission, we are counting
    // on the output being in expected format)
    cout << setw(5) << time << "\t";

    switch (stepAction) {
    case admitNewProc: // move to ready state
      cout << "[  admit]\t";
      break;
    case handleInterrupt: // move to blocked
      cout << "[ inrtpt]\t";
      break;
    case beginRun: // begin running process
      cout << "[  begin]\t";
      break;
    case continueRun: // continue running process
      cout << "[contRun]\t";
      break;
    case ioRequest: // get io for running process
      cout << "[  ioReq]\t";
      break;
    case complete: // process is finished
      cout << "[ finish]\t";
      break;
    case noAct: // default
      cout << "[*noAct*]\t";
      break;
    }

    // You may wish to use a second vector of processes (you don't need to,
    // but you can)

    printProcessStates(processList); // change processList to another vector
                                     // of processes if desired

    this_thread::sleep_for(
        chrono::milliseconds(sleepDuration)); // sleep for 50 seconds
  }

  return 0;
}
