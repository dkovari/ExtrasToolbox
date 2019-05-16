EP = extras.Async.Example.ExampleProcessor; %create processor

CBQ = extras.CallbackQueue; %create callback queue to listen to results from processor

afterEach(CBQ,@CB) %assign callback to the callback queue

lst = addlistener(EP,'ErrorOccured',@(~,err) disp(err)); %add listener for errors

EP.registerQueue(CBQ); %register the callback queue

%% send tasts
for n=1:3
%     fprintf('Press a key to send next task\n\n');
%     pause;
    EP.pushTask('Task',n,rand(10));
end

disp('push key to continue');
pause

disp('pausing processor and adding tasks');
EP.pause();
for n=11:21
    EP.pushTask('Task',n,rand(10));
    fprintf('\tRemainingTasks: %d\n',EP.RemainingTasks);
end
disp('push key to resume');
pause
EP.resume();

%%
disp('push key to continue');
pause
disp('pausing processor and adding tasks before deleting');
disp('delete should create waitbar');
EP.pause();
for n=22:32
    
    EP.pushTask('Task',n,rand(10));
    fprintf('\tRemainingTasks: %d\n',EP.RemainingTasks);
end

disp('push key to delete');
pause
delete(EP);


%% cleanup

delete(CBQ);
delete(lst);


%% define callback function
function CB(data)
fprintf('New Result:\n');
disp(data);
end