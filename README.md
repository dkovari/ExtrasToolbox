# MATextras
Useful data manipulation (non-ui) functions for MATLAB
## Including MATextras in a project
MATextras should be included in a MATLAB project as a package named "+extras".
### Adding as a subtree
```
...In you git repository directory...
# Create a remote connection to MATextras
git remote add -f MATextras https://github.com/dkovari/MATextras/

#Merge into local project
git merge -s ours --no-commit --allow-unrelated-histories MATextras/master

# Copy MATextras into project in a directory named +extras
git read-tree --prefix=+extras/ -u MATextras/master

# Commit the changes
git commit -m "Subtree merged MATextras to +extras"
```
To update the subtree to use the latest commit you need to manually pull changes.
```
git pull -s subtree MATextras master
```
### Adding as a submodule
```
git submodule add https://github.com/dkovari/MATextras/ +extras
#on older versions of git you will also need to pull the updates
git submodule update --init --recursive
```
If you need to update to use the latest version of MATextras then run:
```
git submodule update --init --recursive
```
