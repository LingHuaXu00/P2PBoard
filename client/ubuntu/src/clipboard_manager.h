#ifndef CLIPBOARD_MANAGER_H
#define CLIPBOARD_MANAGER_H

#include <string>
#include <atomic>
#include <chrono>

// X11 headers
#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

// Wayland headers
#ifdef __linux__
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#endif

/**
 * @class ClipboardManager
 * @brief 管理Linux系统剪贴板访问
 *
 * 此类提供跨平台接口用于访问和修改系统剪贴板，
 * 支持X11和Wayland显示服务器。
 * 它处理与剪贴板API交互的底层细节，
 * 同时为应用程序的其他部分提供简单接口。
 */
class ClipboardManager
{
public:
    /**
     * @brief 构造函数
     * 使用默认设置初始化剪贴板管理器
     */
    ClipboardManager();

    /**
     * @brief 析构函数
     * 清理剪贴板管理器使用的资源
     */
    ~ClipboardManager();

    /**
     * @brief 初始化剪贴板管理器
     *
     * 此方法根据当前显示环境设置适当的后端(X11或Wayland)。
     * 应在使用任何其他方法之前调用此方法。
     *
     * @return 初始化成功返回true，否则返回false
     */
    bool initialize();

    /**
     * @brief 获取当前剪贴板内容
     *
     * 此方法从系统剪贴板检索当前文本。
     * 它检查PRIMARY和CLIPBOARD选择，并返回找到的第一个非空内容。
     *
     * @return 当前剪贴板内容作为字符串返回
     */
    std::string get_clipboard_content();

    /**
     * @brief 设置剪贴板内容
     *
     * 此方法将指定文本设置为新的剪贴板内容。
     * 它更新PRIMARY和CLIPBOARD选择，以确保与不同应用程序的兼容性。
     *
     * @param content 要设置到剪贴板中的文本
     */
    void set_clipboard_content(const std::string &content);

public:
    // X11相关成员变量
    Display *x11_display_ = nullptr;
    Atom atom_selection_;
    Atom atom_clipboard_;
    Atom atom_utf8_string_;

    // Wayland相关成员变量
    struct wl_display *wayland_display_ = nullptr;
    struct wl_registry *registry_ = nullptr;
    struct wl_data_device_manager *clipboard_manager_ = nullptr;

    // 标志位指示是否使用Wayland
    bool use_wayland_ = false;

    // 标志位指示管理器是否已初始化
    std::atomic<bool> initialized_ = false;

    // 当前剪贴板内容
    std::string current_content_;

    // 上次检查剪贴板的时间
    std::chrono::steady_clock::time_point last_check_time_;

    // Wayland注册表监听器
    static const struct wl_registry_listener registry_listener_;

    // Wayland数据设备监听器
    static const struct wl_data_device_listener data_device_listener_;

    // 初始化Wayland剪贴板
    void initialize_wayland_clipboard();

    // 获取Wayland剪贴板内容
    std::string get_wayland_clipboard_content();

    // 设置Wayland剪贴板内容
    void set_wayland_clipboard_content(const std::string &content);

    // 初始化X11剪贴板
    void initialize_x11_clipboard();

    // 获取X11剪贴板内容
    std::string get_x11_clipboard_content();

    // 设置X11剪贴板内容
    void set_x11_clipboard_content(const std::string &content);

    // 获取特定X11原子的剪贴板内容
    std::string get_x11_clipboard_content_for_atom(Atom atom);

    // 为特定X11原子设置剪贴板内容
    void set_x11_clipboard_content_for_atom(Atom atom, const std::string &content);
};

#endif // CLIPBOARD_MANAGER_H
