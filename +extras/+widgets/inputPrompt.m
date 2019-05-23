function [answer,canceled] = inputPrompt(varargin)
% Creates input Dialog with type and error checking
% Requires WidgetsToolbox.
%
% Syntax
%   inputPrompt(PromptStruct)
%   inputPrompt(PromptStruct,TitleString)
%   inputPrompt('Prompt',PS,'Title',ts)
%
% PrompStruct should be a struct or object with field/props
%   Label: 'string with label name'
%   Type: 'number'|'text'
%   Value: default value
%   AllowedValues(optional): list of allowed values
%   Limits: [min,max]

%% require uix and uiw
extras.RequireGuiLayoutToolbox;
extras.RequireWidgetsToolbox;

%% Parse Inputs
iH = extras.inputHandler;

iH.addRequiredVariable('Prompt',@(x) isobject(x)&&isprop(x,'Label')||isstruct(x)&&isfield(x,'Label'),true);
iH.addOptionalVariable('Title','Input Prompt',@(x) ischar(x)||isstring(x),true);

iH.parse(varargin{:});

Prompt = iH.Results.Prompt;

OK_Btn = gobjects(1);

%% Setup Answer
canceled = true;

answer = cell(numel(Prompt),1);
if isstruct(Prompt)&&isfield(Prompt,'Value')...
   || isobject(Prompt)&&isprop(Prompt,'Value')

    [answer{:}] = deal(Prompt.Value);

end

answer_type = cell(numel(Prompt),1);
if ~isfield(Prompt,'Type')&&~isprop(Prompt,'Type')
    for n=1:numel(answer)
        if isnumeric(answer{n})&&~isempty(answer{n})
            answer_type{n} = 'number';
        else
            answer_type{n} = 'text';
        end
    end
else
    [answer_type{:}]=deal(Prompt.Type);
end

Prompt_Label = cell(numel(Prompt),1);
if ~isfield(Prompt,'Label')&&~isprop(Prompt,'Label')
    for n=1:numel(Prompt)
        Prompt_Label{n} = sprintf('Prompt %d',n);
    end
else
    [Prompt_Label{:}]=deal(Prompt.Label);
end

    function TF = setAnswer(n,val)
    % Function used to validate and chance answers
        TF = true;
        if isfield(Prompt(n),'AllowedValues')||isprop(Prompt(n),'AllowedValues')
            if ~isempty(Prompt(n).AllowedValues)
                TF = ismember(val,Prompt(n).AllowedValues);
            end
        end
        if strcmpi(answer_type{n},'number')&&(isfield(Prompt(n),'Limits')||isprop(Prompt(n),'Limits'))
            if ~isscalar(val)
                TF=false;
            elseif ~isempty(Prompt(n).Limits)&&numel(Prompt(n).Limits)==2
                LIM = Prompt(n).Limits;
                if isnan(LIM(1))
                    LIM(1) = -Inf;
                end
                if isnan(LIM(2))
                    LIM(2) = -Inf;
                end
                TF = val>=LIM(1)&val<=LIM(2);
                
            end
        end
        if ~TF
            try
                set(OK_Btn,'Enable','off')
            catch
            end
            return;
        end
        answer{n}=val;
    end


%% Create Figure
hFig = figure('Name',iH.Results.Title,...
    'NumberTitle','off',...
    'HandleVisibility','callback',...
    'MenuBar','none',...
    'ToolBar','none');


ovb = uix.VBox('Parent',hFig,'Spacing',2);
sb = uix.ScrollingPanel('Parent',ovb);
pvb = uix.VBox('Parent',sb,'Spacing',1);

ITEM_HEIGHT = 35;
sb.MinimumWidths = 200;
sb.MinimumHeights = ITEM_HEIGHT*numel(answer);

%% create Edits
for n=1:numel(Prompt)
    w = uiw.widget.EditableText(...
        'Parent',pvb,...
        'FieldType',answer_type{n},...
        'Value',answer{n},...
        'Callback',@(h,e) set(h,'TextIsValid',setAnswer(n,h.Value)),...
        'Label',Prompt_Label{n},...
        'LabelLocation','left',...
        'LabelWidth',80,...
        'LabelHorizontalAlignment','right',...
        'TextInvalidBackgroundColor',[1 .8 .8],...
        'TextInvalidForegroundColor',[1,0,0]);
    if strcmpi(answer_type{n},'number')&&(isfield(Prompt(n),'Limits')||isprop(Prompt(n),'Limits'))&&~isempty(Prompt(n).Limits)&&numel(Prompt(n).Limits)==2
        w.LabelTooltipString = sprintf('Limits: [%g , %g]',Prompt(n).Limits);
    end
end
uix.Empty('parent',pvb);
pvb.Heights = [repmat(ITEM_HEIGHT,1,numel(Prompt)),-1];

%% create ok/cancel buttons
hb = uix.HBox('Parent',ovb);
ovb.Heights = [-1,ITEM_HEIGHT];
uix.Empty('Parent',hb);
    
    function HIT_OK()
        canceled = false;
        delete(hFig);
    end

OK_Btn = uicontrol('Parent',hb,...
    'Style','Pushbutton',...
    'String','OK',...
    'Callback',@(~,~) HIT_OK());
uicontrol('Parent',hb,...
    'Style','Pushbutton',...
    'String','Cancel',...
    'Callback',@(~,~) delete(hFig));

hb.Widths = [-1,70,70];

%% move to center of screen
hFig.Position(3:4) = [300,min(300,10+ITEM_HEIGHT*(numel(answer)+1))];
movegui(hFig,'center');
%% wait for fig close
waitfor(hFig);


end