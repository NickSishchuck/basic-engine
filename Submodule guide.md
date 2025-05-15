# Navigate to the submodule directory
cd renderer

# Pull/Push the latest changes
git pull origin main
git push origin main

# Navigate back to the root of the engine
cd ..

# The submodule is now at a different commit, so you need to update your engine repo
git add renderer
git commit -m "Update renderer submodule to latest version"
