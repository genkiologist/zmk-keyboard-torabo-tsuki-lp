if(CONFIG_SHIELD_TORABO_TSUKI_LP_LEFT OR CONFIG_SHIELD_TORABO_TSUKI_LP_RIGHT)
  zephyr_library()
  zephyr_include_directories(${APPLICATION_SOURCE_DIR}/include)
  zephyr_library_sources(src/board.c src/mini_trackpad_init_reg.c)
endif()

# PAW3222 ドライバ（drivers/paw3222/README.md）
if(CONFIG_PAW3222)
  zephyr_library_named(paw3222_driver)
  zephyr_library_sources(drivers/paw3222/src/paw3222.c)
  zephyr_library_include_directories(drivers/paw3222/include)
  zephyr_library_compile_options(-H)  # 一時: インクルード経路の調査
endif()

# Shift で縦／横スクロールを切り替える（central だけで動く）
if(CONFIG_ZMK_INPUT_PROCESSOR_SHIFT_SCROLL_AXIS AND ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL))
  target_sources(app PRIVATE src/input_processors/input_processor_shift_scroll_axis.c)
endif()
