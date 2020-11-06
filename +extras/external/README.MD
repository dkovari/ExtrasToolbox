## +extras/external
This directory contains links/subtrees/submodules to other projects, this library depends on
# Dependencies
* Extras: Submodule pointing to github.com/dkovari/Extras C++ extras library repository.
	- Setup with Junctions to existing Extras .git repo:
	1) after cloning the ExtrasToolbox, go to this directory and replace Extras folder with junction link to the path of the externally cloned Extras repo.
	2) go to ExtrasToolbox/.git/modules/+extras/external/
	3) remove ExtrasToolbox/.git/modules/+extras/external/Extras
	4) replace with a junction link to the location of the .git directory in the cloned Extras repo (e.g. mklink /J "[...]\ExtrasToolbox\.git\modules\+extras\external\Extras" "[...]\Extras\.git"
Junction created for C:\Users\kovardx\OneDrive - Abbott\GitHub\ExtrasToolbox\.git\modules\+extras\external\Extras <<===>> C:\Users\kovardx\OneDrive - Abbott\GitHub\Extras\.git)