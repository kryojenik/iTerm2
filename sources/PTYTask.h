// Implements the interface to the pty session.

#import <Foundation/Foundation.h>

#import "iTermFileDescriptorClient.h"
#import "iTermLoggingHelper.h"
#import "iTermTTYState.h"
#import "VT100GridTypes.h"

#import <termios.h>

@class Coprocess;
@class iTermProcessInfo;
@protocol iTermTask;
@class PTYTab;
@class PTYTask;

@protocol PTYTaskDelegate <NSObject>
// Runs in a background thread. Should do as much work as possible in this
// thread before kicking off a possibly async task in the main thread.
- (void)threadedReadTask:(char *)buffer length:(int)length;

// Runs in the same background task as -threadedReadTask:length:.
- (void)threadedTaskBrokenPipe;
- (void)brokenPipe;  // Called in main thread
- (void)taskWasDeregistered;
- (void)writeForCoprocessOnlyTask:(NSData *)data;

// Called on main thread from within launchWithPath:arguments:environment:customShell:gridSize:viewSize:isUTF8:.
- (void)taskDiedImmediately;

// Main thread
- (void)taskDidChangeTTY:(PTYTask *)task;
@end

typedef NS_ENUM(NSUInteger, iTermJobManagerForkAndExecStatus) {
    iTermJobManagerForkAndExecStatusSuccess,
    iTermJobManagerForkAndExecStatusTempFileError,
    iTermJobManagerForkAndExecStatusFailedToFork,
    iTermJobManagerForkAndExecStatusTaskDiedImmediately
};

typedef NS_ENUM(NSUInteger, iTermJobManagerKillingMode) {
    iTermJobManagerKillingModeRegular,            // SIGHUP, child only
    iTermJobManagerKillingModeForce,              // SIGKILL, child only
    iTermJobManagerKillingModeForceUnrestorable,  // SIGKILL to server if available. SIGHUP to child always.
    iTermJobManagerKillingModeProcessGroup,       // SIGHUP to process group
    iTermJobManagerKillingModeBrokenPipe,         // Removes unix domain socket and file descriptor for it. Ensures server is waitpid()ed on. This does not directly kill the child process.
};

@protocol iTermJobManager<NSObject>

@property (nonatomic) int fd;
@property (nonatomic, copy) NSString *tty;
@property (nonatomic, readonly) pid_t externallyVisiblePid;
@property (nonatomic, readonly) BOOL hasJob;
@property (nonatomic, readonly) id sessionRestorationIdentifier;
@property (nonatomic, readonly) pid_t pidToWaitOn;
@property (nonatomic, readonly) BOOL isSessionRestorationPossible;

- (void)forkAndExecWithTtyState:(iTermTTYState *)ttyStatePtr
                        argpath:(const char *)argpath
                           argv:(const char **)argv
                     initialPwd:(const char *)initialPwd
                     newEnviron:(char **)newEnviron
                    synchronous:(BOOL)synchronous
                           task:(id<iTermTask>)task
                     completion:(void (^)(iTermJobManagerForkAndExecStatus))completion;

- (void)attachToServer:(iTermFileDescriptorServerConnection)serverConnection
         withProcessID:(NSNumber *)thePid
                  task:(id<iTermTask>)task;

- (void)killWithMode:(iTermJobManagerKillingMode)mode;

@end

@interface PTYTask : NSObject<iTermLogging>

@property(atomic, readonly) BOOL hasMuteCoprocess;
@property(atomic, weak) id<PTYTaskDelegate> delegate;

// No reading or writing allowed for now.
@property(atomic, assign) BOOL paused;
@property(nonatomic, readonly) BOOL isSessionRestorationPossible;
@property(nonatomic, readonly) id sessionRestorationIdentifier;

// Tmux sessions are coprocess-only tasks. They have no file descriptor or pid,
// but they may have a coprocess that needs TaskNotifier to read, write, and wait on.
@property(atomic, assign) BOOL isCoprocessOnly;
@property(atomic, readonly) BOOL coprocessOnlyTaskIsDead;

@property(atomic, readonly) int fd;
@property(atomic, readonly) pid_t pid;
// Externally, only PTYSession should assign to this when reattaching to a server.
@property(atomic, readonly) NSString *tty;
@property(atomic, readonly) NSString *path;
@property(atomic, readonly) NSString *getWorkingDirectory;
@property(atomic, readonly) BOOL hasOutput;
@property(atomic, readonly) BOOL wantsRead;
@property(atomic, readonly) BOOL wantsWrite;
@property(atomic, retain) Coprocess *coprocess;
@property(atomic, readonly) BOOL writeBufferHasRoom;
@property(atomic, readonly) BOOL hasCoprocess;
@property(nonatomic, readonly) BOOL passwordInput;
@property(nonatomic) unichar pendingHighSurrogate;
@property(nonatomic, copy) NSNumber *tmuxClientProcessID;

- (instancetype)init;

- (BOOL)hasBrokenPipe;

// Command the profile was created with. nil for login shell or whatever's in the command field of the profile otherwise.
- (NSString *)originalCommand;

- (void)launchWithPath:(NSString*)progpath
             arguments:(NSArray*)args
           environment:(NSDictionary*)env
           customShell:(NSString *)customShell
              gridSize:(VT100GridSize)gridSize
              viewSize:(NSSize)viewSize
                isUTF8:(BOOL)isUTF8
           synchronous:(BOOL)synchronous
            completion:(void (^)(void))completion;

- (void)fetchProcessInfoForCurrentJobWithCompletion:(void (^)(iTermProcessInfo *))completion;
- (iTermProcessInfo *)cachedProcessInfoIfAvailable;

- (void)writeTask:(NSData*)data;

// Cause the slave to receive a SIGWINCH and change the tty's window size. If `size` equals the
// tty's current window size then no action is taken.
- (void)setSize:(VT100GridSize)size viewSize:(NSSize)viewSize;

- (void)stop;

- (void)brokenPipe;
- (void)processRead;
- (void)processWrite;

- (void)stopCoprocess;

// If [iTermAdvancedSettingsModel runJobsInServers] is on, then try for up to
// |timeout| seconds to connect to the server. Returns YES on success.
// If successful, it will be wired up as the task's file descriptor and process.
- (BOOL)tryToAttachToServerWithProcessId:(pid_t)thePid tty:(NSString *)tty;

// Wire up the server as the task's file descriptor and process. The caller
// will have connected to the server to get this info. Requires
// [iTermAdvancedSettingsModel runJobsInServers].
- (void)attachToServer:(iTermFileDescriptorServerConnection)serverConnection;

- (void)killWithMode:(iTermJobManagerKillingMode)mode;

- (void)registerAsCoprocessOnlyTask;
- (void)writeToCoprocessOnlyTask:(NSData *)data;
- (void)getWorkingDirectoryWithCompletion:(void (^)(NSString *pwd))completion;

@end

