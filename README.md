# MATextras
Useful data manipulation (non-ui) functions for MATLAB
## Including MATextras in a project
MATextras should be included in a MATLAB project as a package named "+extras". You can add it to an existing git repository using a subtree.
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
