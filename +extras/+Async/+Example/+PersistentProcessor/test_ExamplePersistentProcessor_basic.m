I = rand(250);
WIND = rand(1,4);

%% get pointer
p = extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('new');

%% push data
extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('setPersistentArgs',p,'Window',WIND);
extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('pushTask',p,I);
pause(1);

remTasks = extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('remainingTasks',p)

nRes = extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('availableResults',p)

nArgOut = extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('numResultOutputArgs',p)
out = cell(1,nArgOut);
[out{:}] =  extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('popResult',p)


%% delete
'press key to delete'
pause
extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('pause',p)
extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('cancelRemainingTasks',p)
remTasks = extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('remainingTasks',p)
extras.Async.Example.PersistentProcessor.ExamplePersistentProcessorMex('delete',p)