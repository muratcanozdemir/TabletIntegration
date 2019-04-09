function update_MySharedMemdll()
run_dir={'D:\progs\Matlab\TabletRecord\Matlab_StateMachine\simulate_EyeSeeParadigms' ...
         'D:\progs\Matlab\StateMachine\simulate_EyeSeeParadigms' ...
         };

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



dll_basedir={'D:\progs\sharedMemDll', ...
	     'D:\sharedMemDll'};
	     
k=0;
while k<=length(dll_basedir),
	k=k+1;
   if k<=length(dll_basedir),
      if exist(dll_basedir{k},'dir')==7,
         break;
      end;
   end;
end;
if k>length(dll_basedir),
   error('dll base-directory not found!');
end;
dll_basedir=dll_basedir{k};
	     

dll_name='MySharedDLL';
dll_dirname=[dll_basedir,'\Debug'];
dll_header_dirname=[dll_basedir,'\sharedMemDll'];
mat_dll_dirname={'D:\progs\SharedMemory\MATLAB_DLL' ...
                ,run_dir ...
   };
usr_dll_dirname='D:\progs\SharedMemory';

for k=1:length(mat_dll_dirname),
   copyfile([dll_dirname,'\',dll_name,'.dll'],[mat_dll_dirname{k},'\',dll_name,'.dll']);
   copyfile([dll_header_dirname,'\',dll_name,'.h'],[mat_dll_dirname{k},'\',dll_name,'.h']);
end;

copyfile([dll_dirname,'\',dll_name,'.dll'],[usr_dll_dirname,'\',dll_name,'.dll']);
copyfile([dll_dirname,'\',dll_name,'.lib'],[usr_dll_dirname,'\',dll_name,'.lib']);
copyfile([dll_header_dirname,'\',dll_name,'.h'],[usr_dll_dirname,'\',dll_name,'.h']);

end
