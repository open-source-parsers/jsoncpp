set -vex

#brew update
brew upgrade python3
python3 -m venv venv
source venv/bin/activate

brew install ninja
brew install meson

#brew install pyenv
#which pyenv
