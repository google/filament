SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

for DIR in "Device" "Pipeline" "Reactor" "System" "Vulkan" "WSI"
do
    # Double clang-format, as it seems that one pass isn't always enough
    find ${SRC_DIR}/$DIR  -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.inl" | xargs clang-format -i
    find ${SRC_DIR}/$DIR  -iname "*.hpp" -o -iname "*.cpp" -o -iname "*.inl" | xargs clang-format -i

    git add ${SRC_DIR}/$DIR
    CHANGE_ID="$(echo $CHANGE_ID_SEED $DIR | openssl sha1)"
    git commit -m "clang-format the src/$DIR directory" -m "Bug: b/144825072" -m "Change-Id:I$CHANGE_ID"
done

