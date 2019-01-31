function YX =imtrace(IM,THRESHOLD)
% Trace along neighboring pixels in an image.
%
% Routine written by Haowei Wang.
% This function is just a wrapper around Haowei's original tracing routine,
% making it a little easier to use with other functions.
%
% Usage:
%   XY = imtrace(IM,THRESHOLD)
%
% Inputs:
%   IM: The image that should be traced.
%   
%   THRESHOLD: the value which specifies the level used to threshold IM
%   into a binary image.
%
% Output:
%   YX: A cell array where each cell specifies the coordinates of a
%   traced segment.
%   Segments are specified as column vectors:
%       YX = [Y1,X1;
%             Y2,X2;...]
%   Note: There appears to be a pixel-shift of +1 for each coodinate
%
%% Copyright 2017 Emory University
% All rights reserved.

%% Change Log
% 2018-10-25: Dan Kovari, Emory U.
%   changed name and moved into extras.ImageProcessing package
% 2017-01-31: Dan Kovari, Emory U.
%   Initial file creation, based on tracing_v2()


%% 
ReMax=5000; % the maximum number of points on one tracing
    Record=zeros(ReMax,2);
    minLength = 10; % set the minimum length for a trace
    
traceNum=0;
    
    test{1}=[-1 -1 0; 0 -1 0; 0 0 0];
    test{2}=[0 -1 -1; 0 -1 0; 0 0 0];
    test{3}=[0 0 -1; 0 -1 -1; 0 0 0];
    test{4}=[0 0 0; 0 -1 -1; 0 0 -1];
    test{5}=[0 0 0; 0 -1 0; 0 -1 -1];
    test{6}=[0 0 0; 0 -1 0; -1 -1 0];
    test{7}=[0 -1 0; 0 -1 -1; 0 0 0];
    test{8}=[0 0 0; 0 -1 -1; 0 -1 0];
    %left test arrays
    
    image=IM;
    
    Msize=size(image);
    mask=zeros(Msize);
    
%     biColor=image;
%     biColor(biColor<(OVERWHELM-BASE)*THRESHOLD+BASE)=0;
%     %     biColor(biColor>OVERWHELM)=0;
%     biColor(biColor>0)=1;
biColor = double(IM>THRESHOLD);
    
    biColor(:,1)=0;
    biColor(:,Msize(2))=0;
    biColor(:,Msize(2)-1)=0;
    biColor(1,:)=0;
    biColor(Msize(1),:)=0;
    biColor(Msize(1)-1,:)=0;
    %clear the edge
    
    modify=-1;
    
    while (modify<0)
        modify=0;
        for i=1:4
            for j=2:Msize(2)-1
                for k=2:Msize(1)-1
                    if (biColor(k,j)>0)
                        if(biColor(k,j-1)==0)
                            around=biColor(k-1:k+1,j-1:j+1);
                            sumR=sum(sum(around));
                            if (sumR>3)
                                mask(k,j)=-1;
                                if (around(1,1)==1)
                                    if (around(1,2)==0)
                                        mask(k,j)=0;
                                    end
                                end
                                if (around(1,2)==1)
                                    if (around(1,1)+around(1,3)+around(2,3)==0)
                                        mask(k,j)=0;
                                    end
                                end
                                if (around(1,3)==1)
                                    if(around(1,2)+around(2,3)==0)
                                        mask(k,j)=0;
                                    end
                                end
                                if (around(2,3)==1)
                                    if(around(1,2)+around(1,3)+around(3,2)+around(3,3)==0)
                                        mask(k,j)=0;
                                    end
                                end
                                if (around(3,3)==1)
                                    if (around(2,3)+around(3,2)==0)
                                        mask(k,j)=0;
                                    end
                                end
                                if (around(3,2)==1)
                                    if (around(3,1)+around(2,3)+around(3,3)==0)
                                        mask(k,j)=0;
                                    end
                                end
                                if (around(3,1)==1)
                                    if (around(3,2)==0)
                                        mask(k,j)=0;
                                    end
                                end
                                
                                if(sumR>4)
                                    if (around(2,3)==0)
                                        mask(k,j)=0;
                                    end
                                    
                                end
                                % This part preserve the pixels that can break the skeleton if be removed
                                if sumR==4
                                    AdjTest=around(1,1)*around(1,2)+around(1,2)*around(1,3);
                                    AdjTest=AdjTest+around(1,3)*around(2,3)+around(2,3)*around(3,3);
                                    AdjTest=AdjTest+around(3,3)*around(3,2)+around(3,2)*around(3,1);
                                    if AdjTest==0
                                        mask(k,j)=-1;
                                    end
                                end
                                
                                % This part removes the pixels split the skeleton
                                
                            elseif (sumR==3)
                                for l=1:8
                                    testAround=abs(around+test{l});
                                    if (sum(sum(testAround))==0)
                                        mask(k,j)=-1;
                                        break;
                                    end
                                end
                            end
                            %                         if (mask(k,j)==-1)
                            %                             around
                            %                         end
                            
                        end
                        
                    end
                end
            end
            
            
            biColor=biColor+mask;
            modify=modify+sum(sum(mask));
            Msize=Msize*[0 1; 1 0];
            biColor=rot90(biColor);
            mask=zeros(Msize);
            %rotate 90 degree to repeat
        end
    end
        
YX = {};
        for i=2:Msize(1)-1
            for j=2:Msize(2)-1
                if biColor(i,j)==1
                    around=biColor(i-1:i+1,j-1:j+1);
                    if sum(sum(around))==2
                        Record(1,1)=i;
                        Record(1,2)=j;
                        Record(1,:)=Record(1,:);
                        traceNum=traceNum+1;
                        YX{traceNum} = [];
                        m=2;
                        k=i;
                        l=j;
                        testNext=[1 2 3; 4 5 6; 7 8 9];
                        while (sum(sum(around))>1)
                            biColor(k,l)=0;
                            around(2,2)=0;
                            tNext=sum(sum(testNext.*around));
                            
                            switch tNext
                                case 1
                                    k=k-1; l=l-1;
                                case 2
                                    k=k-1;
                                case 3
                                    k=k-1; l=l+1;
                                case 4
                                    l=l-1;
                                case 6
                                    l=l+1;
                                case 7
                                    k=k+1; l=l-1;
                                case 8
                                    k=k+1;
                                case 9
                                    k=k+1; l=l+1;
                                otherwise
                                    %                                     traceNum=traceNum-1;
                                    Record=zeros(ReMax,2);
                                    break;
                            end
                            Record(m,1)=k;
                            Record(m,2)=l;
                            Record(m,:)=Record(m,:);
                            m=m+1;
                            if m==ReMax
                                break;
                            end
                            around=biColor(k-1:k+1,l-1:l+1);
                        end
                        biColor(k,l)=0;
                        if m<minLength
                            traceNum=traceNum-1;
                            Record=zeros(ReMax,2);
                        elseif  Record(1,1)+Record(1,2)==0
                            m=1;
                        else
                            %fprintf(fd, '%d %d \n\r', -1, traceNum);
                            for n=1:ReMax
                                if Record(n,1)+Record(n,2)==0
                                    %fprintf(fd, '%d %d \n\r', -1, 0);
                                    break;
                                end
%                                fprintf(fd, '%d %d \n\r', Record(n,:));
                                YX{traceNum}=[YX{traceNum};Record(n,:)];
                            end
                            Record=zeros(ReMax,2);
                        end
                    end
                end
            end
        end


