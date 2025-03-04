# Updating gpu_info.json

Dawn uses `gpu_info.json` to group GPUs by vendor and architecture when calling `adapter.info`. It uses
vendor and masked PCI device IDs to identify the GPUs, and as such will need to be updated from time to time as new
devices come on the market.

## Guidelines for adding a new device or set of devices

There are no automated processes to add devices to `gpu_info.json`. Everything is inserted by hand. Fortunately the
file is intended to be human readable, and should be relatively easy to update following the comments at the top of the
file and referencing the existing entries.

When adding new devices use of a `mask` is encouraged when applicable to capture more devices with less entries.
However, it's important to ensure that the masked device IDs are not overly broad or capture devices which do not belong
to a given architecture. Many GPU vendors do not use very consistent patterns in their IDs, or have strange exceptions
to the patterns that do exist.

When in doubt, prefer adding an explicit device ID without a `mask` over adding an masked ID that might accidentally
capture more devices than intended.

A GPUs `architecture` should be the vendor's terminology for a given family of devices when possible. An `architecture`
group should not be detailed enough to identify specific devices, but should ideally group all devices made within a
span of a couple of years or longer. It is OK to have significant performance disparities between devices in a single
`architecture` group, as long as the capabilities and behavior of the devices are roughly similar.

## Rolling new devices

Capturing new devices that need to be added to `gpu_info.json` right now is unfortunately a fairly awkward process. The
method used so far has been to checkout https://github.com/kainino0x/gpuinfo-vulkan-query, update the data submodule
with the latest records from https://gpuinfo.org using the script at https://github.com/kainino0x/gpuinfo-vulkan-slurp
(Ask kainino@ or bajones@ for access), and then run `device_id.py` with the path to the `gpu_info.json` file you are
updating. That script will compare all the records to the entries in `gpu_info.json` and print out how each of them are
categorized followed by a list of all the uncategorized device IDs and descriptions sorted by vendor. You'll have to
manually look at the device descriptions and research their architectural design in order to categorize them.

Good luck!

