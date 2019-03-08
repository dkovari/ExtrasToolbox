
try
    delete(EP2)
catch
end

EP2 = extras.Async.Example.ExampleProcessor_ParamArgs; %create processor

CBQ = extras.CallbackQueue; %create callback queue to listen to results from processor

afterEach(CBQ,@CB) %assign callback to the callback queue

lst = addlistener(EP2,'ErrorOccured',@(~,err) disp(err)); %add listener for errors

EP2.registerQueue(CBQ); %register the callback queue

%% send tasts
for n=1:3
%     fprintf('Press a key to send next task\n\n');
%     pause;
    EP2.pushTask('Task',n,rand(10));
end

disp('push key to continue');
pause

disp('pausing processor and adding tasks and persistent args');
EP2.pause();
EP2.setParameters('Persist',[100,200,300]);

for n=11:21
    EP2.pushTask('Task',n,rand(10));
    fprintf('\tRemainingTasks: %d\n',EP2.RemainingTasks);
end
disp('push key to resume');
pause
EP2.resume();

%%
disp('push key to continue');
pause
disp('pausing processor and adding tasks before deleting');
disp('delete should create waitbar');
EP2.pause();
for n=22:35
    
    EP2.pushTask('Task',n,rand(10));
    fprintf('\tRemainingTasks: %d\n',EP2.RemainingTasks);
end

disp('push key to delete');
pause
delete(EP2);


%% cleanup

delete(CBQ);
delete(lst);


%% define callback function
function CB(data)
fprintf('New Result:\n');
disp(data);
end