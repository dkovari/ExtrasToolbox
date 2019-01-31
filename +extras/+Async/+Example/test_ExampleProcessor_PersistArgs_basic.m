I = rand(250);
WIND = rand(1,4);

%% get pointer
p = extras.Async.ExampleProcessor_PersistArgsMex('new');

%% push data
extras.Async.ExampleProcessor_PersistArgsMex('pushTask',p,I,WIND);
extras.Async.ExampleProcessor_PersistArgsMex('pushTask',p,I,WIND);
pause(1);

remTasks = extras.Async.ExampleProcessor_PersistArgsMex('remainingTasks',p)

nRes = extras.Async.ExampleProcessor_PersistArgsMex('availableResults',p)

%% delete
'press key to delete'
pause
extras.Async.ExampleProcessor_PersistArgsMex('pause',p)
extras.Async.ExampleProcessor_PersistArgsMex('cancelRemainingTasks',p)
remTasks = extras.Async.ExampleProcessor_PersistArgsMex('remainingTasks',p)
extras.Async.ExampleProcessor_PersistArgsMex('delete',p)