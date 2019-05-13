
try
    delete(EP3)
catch
end

EP3 = extras.Async.Example.ParamProcessor.ExampleParamProcessor; %create processor

EP_UI = extras.Async.AsyncProcessorWithWriterUI(EP3);

CBQ = extras.CallbackQueue; %create callback queue to listen to results from processor

afterEach(CBQ,@CB) %assign callback to the callback queue

lst = addlistener(EP3,'ErrorOccured',@(~,err) disp(err)); %add listener for errors

EP3.registerQueue(CBQ); %register the callback queue




%% send tasts
for n=1:3
     fprintf('Press a key to send next task\n\n');
     pause;
    EP3.pushTask('Task',n,rand(10));
end

disp('push key to continue');
pause

disp('pausing processor and adding tasks and persistent args');
EP3.pause();
EP3.setParameters('Persist',[100,200,300]);

for n=11:21
    EP3.pushTask('Task',n,rand(10));
    fprintf('\tRemainingTasks: %d\n',EP3.RemainingTasks);
end
disp('push key to resume');
pause
EP3.resume();

%%
disp('push key to continue');
pause
disp('pausing processor and adding tasks before deleting');
disp('delete should create waitbar');
%EP3.pause();
for n=22:35
    
    EP3.pushTask('Task',n,rand(10));
    fprintf('\tRemainingTasks: %d\n',EP3.RemainingTasks);
end

disp('push key to delete');
pause
delete(EP3);


%% cleanup

delete(CBQ);
delete(lst);


%% define callback function
function CB(data)
fprintf('New Result:\n');
disp(data);
end