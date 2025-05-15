# Git Submodule Guide

## Update an existing submodule
___
```bash
# Navigate to the submodule directory
cd renderer
# Pull the latest changes
git pull origin main
# Navigate back to the root of the engine
cd ..
# The submodule is now at a different commit, so you need to update your engine repo
git add renderer
git commit -m "Update renderer submodule to latest version"
```

## Clone a repository with submodules
___
```bash
git clone --recurse-submodules https://github.com/username/engine.git
# If you already cloned without --recurse-submodules
git submodule init
git submodule update
```

## Add a new submodule
___
```bash
git submodule add https://github.com/username/new-module.git path/to/new-module
git commit -m "Add new-module submodule"
```

## Update all submodules at once
___
```bash
# Update all submodules to their latest commits
git submodule update --remote --merge
# Alternatively, pull all submodules recursively
git pull --recurse-submodules
```

## Check submodule status
___
```bash
# See which submodules have changes
git submodule status
# For more detailed info
git submodule foreach 'git status'
```

## Remove a submodule
___
```bash
# Remove the submodule entry from .git/config
git submodule deinit -f path/to/submodule
# Remove the submodule from the working tree and index
git rm -f path/to/submodule
# Remove the submodule directory from .git/modules
rm -rf .git/modules/path/to/submodule
# Commit the change
git commit -m "Remove submodule"
```

### Difference between updating and pulling a submodule

**Git Pull**: When you run `git pull` inside a submodule, it fetches changes from the remote repository and merges them into the current branch of the submodule. This is like running `git fetch` followed by `git merge`. This command:
- Works on the current branch
- Merges new changes
- Is executed inside the submodule directory

**Git Submodule Update**: When you run `git submodule update` from the parent repository, it checks out the exact commit that the parent repository is pointing to. This command:
- Doesn't merge changes, just moves to a specific commit
- Often puts the submodule in a detached HEAD state
- Is executed from the parent repository
- Ensures the submodule matches what the parent repo expects
