function run_eyesee()
run_dir={ ...
          'D:\progs\Matlab\TabletRecord\Matlab_StateMachine\simulate_EyeSeeParadigms' ...
         ,'D:\progs\Matlab\StateMachine\simulate_EyeSeeParadigms' ...
         ,'D:\progs\Matlab\StateMachine\simulate_EyeSeeParadigms' ...
         };
mat_dir={'D:\progs\Matlab' ...
         'D:\Matlab2017b' ...
         'D:\Matlab' ...
         };
DataDirname='D:\Data\Exp1';
Subdirname='dummy';
DataDirname=[DataDirname,'\',Subdirname];
if exist(DataDirname,'dir')~=7,
   mkdir(DataDirname);
end;


k=0;
while k<=length(run_dir),
	k=k+1;
   if k<=length(run_dir),
      if exist(run_dir{k},'dir')==7,
         break;
      end;
   end;
end;
if k>length(run_dir),
   error('run directory not found!');
end;
run_dir=run_dir{k};

k=0;
while k<=length(mat_dir),
	k=k+1;
   if k<=length(mat_dir),
      if exist(mat_dir{k},'dir')==7,
         break;
      end;
   end;
end;
if k>length(mat_dir),
   error('mat directory not found!');
end;
mat_dir=mat_dir{k};


%*** setup the path 
cd(mat_dir);
matlabpath(pathdef);
      %** add particular directories **
p=[ ...
    run_dir,'\..\..\..\MATC\Fit_dist;' ...
   ];
path( p, path);
      %********************************
addpsych_path;
cd(run_dir);
fprintf('MATLAB READY!\n');
if exist('Paradigm_Name','file')==2,
   delete('Paradigm_Name.m');
   clear fun Paradigm_Name
end;

save -v4 DataDirName.mat DataDirname   

include_INIT_EXIT_in_LOOP=false;
if ~include_INIT_EXIT_in_LOOP,
   script_name='GeneralScript';
   %init_opengl();
   rc=prepend_directories_on_path();
   if rc~=0,
   %   Screen('CloseAll')
      return;
   end;
   
   
   disp([script_name,'(''ExaminationInit'',''0'');']);
   eval([script_name,'(''ExaminationInit'',''0'');']);
end;   


while 1,
   if exist('Paradigm_Name','file')==2,
      pause(0.2);
      ParadigmName=[];
      Paradigm_Name
      delete('Paradigm_Name.m');
      clear fun Paradigm_Name
      
      
      
      if ~isempty(ParadigmName),
         fprintf('ParadigmName=%s\n',ParadigmName);
         if strcmp(ParadigmName,'exit'),
            break;
         elseif strcmp(ParadigmName,'SetDataDirectory'),
            DataDirname=edit_box('Subdirectory',DataDirname);
            if exist(DataDirname,'dir')~=7,
               mkdir(DataDirname);
            end;
            save -v4 DataDirName.mat DataDirname
         else   
            %test_MySharedDLL_fun(false);
            run_eyesee_paradigm('',ParadigmName,include_INIT_EXIT_in_LOOP);
         end;
      else
         pause(0.2);
      end;
      
      
   else
      pause(0.2);
   end;
end;
if exist('DataDirName.mat','file')==2,
   delete('DataDirName.mat');
end;

if ~include_INIT_EXIT_in_LOOP,
   disp([script_name,'(''ExaminationExit'',''0'');']);
   eval([script_name,'(''ExaminationExit'',''0'');']);
   exit_opengl();
end;   

exit



end

function rc = init_opengl ()
    % ################## Initialize OpenGL window ######################
    % ------new-----ohne warnung-----
    
    no_init_with_one_screen = 0;
    init_open_gl_by_script = 0;

    screenid = max(Screen('Screens'))
    %screenid=0 %STANI 
    if(screenid==0 && no_init_with_one_screen)
        save my_workspace.mat no_init_with_one_screen init_open_gl_by_script;
        return;
    end; 
    
    save my_workspace screenid
    which('my_workspace.mat')
    
    
    try
        
        %%% use Screen: mode=0 ; use opengl: mode=1;
        rc_screen = initStimulusScreen ( 0 );

        load my_workspace window_h;
        x_pos = 500;
        y_pos = 500;
        if ~isunix 
            %Screen('TextFont', window_h, 'Arial');%9x15bold
            Screen('TextSize', window_h, 50);
            %Screen('TextStyle', window_h, 0);
        end;
        Screen('DrawText', window_h, 'EyeSeeCam', x_pos, y_pos, [255, 255, 255]);
        Screen('Flip', window_h);
    
    catch
    end;
    
    
    save my_workspace -append no_init_with_one_screen init_open_gl_by_script;
    
end

function rc = exit_opengl ()
    
    try
        Priority (0);
        % Exit
        %glClear;
        %Screen('Flip', window_var);
        Screen ('CloseAll');
    catch
    end;
    rc=0;
end
