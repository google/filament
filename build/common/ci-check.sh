echo "This script is intended to run in a CI environment and may modify your current environment."
echo "Please refer to BUILDING.md for more information."

read -r -p "Do you wish to proceed (y/n)? " choice
case "${choice}" in
    y|Y)
	      echo "Build will proceed..."
	      ;;
    n|N)
    	  exit 0
    	  ;;
	  *)
        exit 0
        ;;
esac

if [[ "$GITHUB_WORKFLOW" ]]; then
    echo "Running workflow $GITHUB_WORKFLOW (event: $GITHUB_EVENT_NAME, action: $GITHUB_ACTION)"
fi
