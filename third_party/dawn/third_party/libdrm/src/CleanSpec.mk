$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/include/libdrm)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/include/freedreno)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libdrm_*intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/STATIC_LIBRARIES/libdrm_*intermediates)
