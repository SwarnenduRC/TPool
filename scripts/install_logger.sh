#!/usr/bin/env bash
#set -e  # Exit immediately if a command exits with a non-zero status

# First get the type of the logger library requested by the user to be used
# i.e.; static or shared

echo "LOGGER_LIB_TYPE=$LOGGER_LIB_TYPE"
LIB_TYPE="$LOGGER_LIB_TYPE"

echo "Entering logger installation script..."

if [[ -z "$LIB_TYPE" ]]; then
    echo "Logger lib type is not specified"
    echo "static attachment is considered as default"
    LIB_TYPE="static"
fi

clone_and_install_liblogger() {
    LIB_LOGGER_REPO_URL="https://github.com/SwarnenduRC/Logger.git"
    TARGET_DIR="LOGGER"
    git clone "$LIB_LOGGER_REPO_URL" "$TARGET_DIR"
    if [ 0 -eq $? ] && [ -d "$TARGET_DIR/.git" ]; then
        echo "Cloning of Liblogger library into $TARGET_DIR directory successful"
        echo "Now installing..."
        echo ""
        cd "$TARGET_DIR"
        # If the logging is file logging then call the buildNInstall script
        # of the Logger lib with related file attributes
        if [[ "$FILE_LOGGING" == "yes" ]]; then
            ./buildNinstall.sh -BUILD_TYPE="release" -FILE_LOGGING="$FILE_LOGGING" -FILE_SIZE="$FILE_SIZE" \
            -FILE_SIZE="$FILE_SIZE" -LOG_FILE_PATH="$LOG_FILE_PATH" -LOG_FILE_NAME="$LOG_FILE_NAME" \
            -LOG_FILE_EXTN="$LOG_FILE_EXTN" -LIB_TYPE="$LIB_TYPE"
        else
            ./buildNinstall.sh -BUILD_TYPE="release" -LIB_TYPE="$LIB_TYPE"
        fi

        # Now check if the installation is successful or not
        if [[ "$LIB_NAME_TO_FIND" == "liblogger.a" ]]; then
            FOUND=$(find /usr/local/lib/ -maxdepth 1 -type f -name "*.a" | grep -i "$LIB_NAME_TO_FIND" || true);
        else
            FOUND=$(find /usr/local/lib/ -maxdepth 1 -type f -name "*.so" | grep -i "$LIB_NAME_TO_FIND" || true);
        fi

        if [[ "$FOUND" != "" ]]; then
            echo "Installation of Logger library successful"
            echo "Cleaning the logger directory now...."
            cd ../
            rm -rf "$TARGET_DIR/*"
            rm -rf "$TARGET_DIR"
        else
            echo "Installation of Logger library unsuccessful"
            echo "Cleaning the logger directory now and exiting after marking FAILURE...."
            rm -rf "$TARGET_DIR/*"
            rm -rf "$TARGET_DIR"
            exit -1
        fi
    fi
}

LIB_NAME_TO_FIND=""
if [[ "$LIB_TYPE" == "static" ]]; then
    LIB_NAME_TO_FIND="liblogger.a"
else
    LIB_NAME_TO_FIND="liblogger.so"
fi

FOUND=""

if [[ "$LIB_NAME_TO_FIND" == "liblogger.a" ]]; then
    FOUND=$(find /usr/local/lib/ -maxdepth 1 -type f -name "*.a" | grep -i "$LIB_NAME_TO_FIND" || true);
else
    FOUND=$(find /usr/local/lib/ -maxdepth 1 -type f -name "*.so" | grep -i "$LIB_NAME_TO_FIND" || true);
fi

if [[ -z "$FOUND" ]]; then
    echo "Logger library not found in the system. Cloning and installing now..."
    clone_and_install_liblogger
else
    echo "Logger library found in the system. Nothing to do here!! :-)"
fi

exit 0