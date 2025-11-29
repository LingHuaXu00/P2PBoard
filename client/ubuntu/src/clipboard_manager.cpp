#include "clipboard_manager.h"
#include "config.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

// 构造函数
ClipboardManager::ClipboardManager()
{
    // 初始化剪贴板访问
    initialized_ = false;
    current_content_ = "";
    last_check_time_ = std::chrono::steady_clock::now();
}

// 析构函数
ClipboardManager::~ClipboardManager()
{
    // 如有必要清理资源
}

// 初始化剪贴板管理器
bool ClipboardManager::initialize()
{
    try
    {
        // 检查我们是否在X11或Wayland上运行
        const char *display_env = std::getenv("WAYLAND_DISPLAY");
        const char *x11_display_env = std::getenv("DISPLAY");

        if (display_env && strlen(display_env) > 0)
        {
            // 在Wayland上运行
            std::cout << "在Wayland上运行。使用Wayland剪贴板API。" << std::endl;
            use_wayland_ = true;
        }
        else if (x11_display_env && strlen(x11_display_env) > 0)
        {
            // 在X11上运行
            std::cout << "在X11上运行。使用X11剪贴板API。" << std::endl;
            use_wayland_ = false;
        }
        else
        {
            // 回退到X11
            std::cout << "未检测到显示环境。默认使用X11剪贴板API。" << std::endl;
            use_wayland_ = false;
        }

        // 初始化适当的剪贴板后端
        if (use_wayland_)
        {
            initialize_wayland_clipboard();
        }
        else
        {
            initialize_x11_clipboard();
        }

        // 成功
        initialized_ = true;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "初始化剪贴板管理器失败: " << e.what() << std::endl;
        return false;
    }
}

// 获取当前剪贴板内容
std::string ClipboardManager::get_clipboard_content()
{
    if (!initialized_)
    {
        std::cerr << "剪贴板管理器未初始化。无法获取剪贴板内容。" << std::endl;
        return "";
    }

    try
    {
        std::string current_content = "";

        if (use_wayland_)
        {
            current_content = get_wayland_clipboard_content();
        }
        else
        {
            current_content = get_x11_clipboard_content();
        }

        // 检查内容自上次检查以来是否发生变化
        if (current_content != current_content_)
        {
            current_content_ = current_content;
            last_check_time_ = std::chrono::steady_clock::now();
        }

        return current_content;
    }
    catch (const std::exception &e)
    {
        std::cerr << "读取剪贴板内容时出错: " << e.what() << std::endl;
        return "";
    }
}

// 设置剪贴板内容
void ClipboardManager::set_clipboard_content(const std::string &content)
{
    if (!initialized_)
    {
        std::cerr << "剪贴板管理器未初始化。无法设置剪贴板内容。" << std::endl;
        return;
    }

    try
    {
        if (use_wayland_)
        {
            set_wayland_clipboard_content(content);
        }
        else
        {
            set_x11_clipboard_content(content);
        }

        // 更新本地副本
        current_content_ = content;
        last_check_time_ = std::chrono::steady_clock::now();
    }
    catch (const std::exception &e)
    {
        std::cerr << "设置剪贴板内容时出错: " << e.what() << std::endl;
    }
}

// 初始化Wayland剪贴板
void ClipboardManager::initialize_wayland_clipboard()
{
    // 创建到Wayland合成器的连接
    wayland_display_ = wl_display_connect(nullptr);
    if (!wayland_display_)
    {
        throw std::runtime_error("连接Wayland显示失败");
    }

    // 获取注册表
    registry_ = wl_display_get_registry(wayland_display_);
    if (!registry_)
    {
        throw std::runtime_error("获取Wayland注册表失败");
    }

    // 注册全局对象
    wl_registry_add_listener(registry_, &registry_listener_, this);

    // 获取剪贴板管理器
    wl_display_roundtrip(wayland_display_);

    // 检查剪贴板管理器是否可用
    if (!clipboard_manager_)
    {
        throw std::runtime_error("Wayland剪贴板管理器不可用");
    }

    // 注册剪贴板事件
    // wl_data_device_manager_add_listener(clipboard_manager_, &data_device_listener_, this);
    wl_display_roundtrip(wayland_display_);

    std::cout << "Wayland剪贴板管理器初始化成功" << std::endl;
}

// 获取Wayland剪贴板内容
std::string ClipboardManager::get_wayland_clipboard_content()
{
    // 这是一个简化的实现
    // 在实际应用中，您需要处理完整的Wayland协议

    // 目前，我们将使用一个虚拟实现
    // 在实践中，您需要实现完整的Wayland剪贴板协议
    // 这涉及创建数据设备、处理选择等

    // 返回测试字符串
    return "来自Wayland的测试剪贴板内容";
}

// 设置Wayland剪贴板内容
void ClipboardManager::set_wayland_clipboard_content(const std::string &content)
{
    // 这是一个简化的实现
    // 在实际应用中，您需要实现完整的Wayland协议

    // 目前，我们只打印一条消息
    std::cout << "设置Wayland剪贴板内容: " << content << std::endl;
}

// 初始化X11剪贴板
void ClipboardManager::initialize_x11_clipboard()
{
    // 连接到X11服务器
    x11_display_ = XOpenDisplay(nullptr);
    if (!x11_display_)
    {
        throw std::runtime_error("连接X11显示失败");
    }

    // 获取原子
    atom_selection_ = XInternAtom(x11_display_, "PRIMARY", False);
    atom_clipboard_ = XInternAtom(x11_display_, "CLIPBOARD", False);
    atom_utf8_string_ = XInternAtom(x11_display_, "UTF8_STRING", False);

    // 检查X11是否可用
    if (atom_selection_ == 0 || atom_clipboard_ == 0 || atom_utf8_string_ == 0)
    {
        throw std::runtime_error("X11原子不可用");
    }

    std::cout << "X11剪贴板管理器初始化成功" << std::endl;
}

// 获取X11剪贴板内容
std::string ClipboardManager::get_x11_clipboard_content()
{
    try
    {
        // 尝试PRIMARY和CLIPBOARD选择
        std::string content = get_x11_clipboard_content_for_atom(atom_selection_);

        if (content.empty())
        {
            content = get_x11_clipboard_content_for_atom(atom_clipboard_);
        }

        return content;
    }
    catch (const std::exception &e)
    {
        std::cerr << "读取X11剪贴板时出错: " << e.what() << std::endl;
        return "";
    }
}

// 设置X11剪贴板内容
void ClipboardManager::set_x11_clipboard_content(const std::string &content)
{
    try
    {
        // 设置PRIMARY和CLIPBOARD选择
        set_x11_clipboard_content_for_atom(atom_selection_, content);
        set_x11_clipboard_content_for_atom(atom_clipboard_, content);
    }
    catch (const std::exception &e)
    {
        std::cerr << "设置X11剪贴板时出错: " << e.what() << std::endl;
    }
}

// 获取特定X11原子的剪贴板内容
std::string ClipboardManager::get_x11_clipboard_content_for_atom(Atom atom)
{
    // 获取拥有选择的窗口
    Window window = XGetSelectionOwner(x11_display_, atom);

    if (window == 0)
    {
        return ""; // 没有所有者，所以没有数据
    }

    // 请求数据
    XSetSelectionOwner(x11_display_, atom, window, CurrentTime);

    // 等待数据可用
    XEvent event;
    bool got_data = false;

    while (!got_data)
    {
        XNextEvent(x11_display_, &event);

        switch (event.type)
        {
        case SelectionNotify:
        {
            // 检查选择是否成功
            if (event.xselection.property != None)
            {
                // 获取数据
                Atom actual_type;
                int actual_format;
                unsigned long nitems;
                unsigned long bytes_after;
                unsigned char *data;

                XGetWindowProperty(x11_display_, window, event.xselection.property,
                                   0, 0, 0, XA_STRING, &actual_type, &actual_format,
                                   &nitems, &bytes_after, &data);

                if (actual_type == atom_utf8_string_ && data)
                {
                    std::string result((char *)data, nitems);
                    XFree(data);
                    got_data = true;
                    return result;
                }
            }
            break;
        }
        case SelectionRequest:
        {
            // 处理选择请求
            XEvent reply;
            memset(&reply, 0, sizeof(reply));
            reply.type = SelectionNotify;
            reply.xselection.requestor = event.xselectionrequest.requestor;
            reply.xselection.selection = event.xselectionrequest.selection;
            reply.xselection.target = event.xselectionrequest.target;
            reply.xselection.property = event.xselectionrequest.property;
            reply.xselection.time = event.xselectionrequest.time;

            // 发送回数据
            XSendEvent(x11_display_, event.xselectionrequest.requestor, False, 0, &reply);
            break;
        }
        }
    }

    return "";
}

// 为特定X11原子设置剪贴板内容
void ClipboardManager::set_x11_clipboard_content_for_atom(Atom atom, const std::string &content)
{
    // 创建一个窗口来保存选择
    Window window = XCreateSimpleWindow(x11_display_, RootWindow(x11_display_, 0),
                                        0, 0, 1, 1, 0, 0, 0);

    // 设置选择所有者
    XSetSelectionOwner(x11_display_, atom, window, CurrentTime);

    // 等待选择被设置
    XFlush(x11_display_);

    // 保持窗口存活直到选择被处理
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 清理
    XDestroyWindow(x11_display_, window);
    XFlush(x11_display_);
}

// Wayland注册表监听器回调
static void registry_handle_global(void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface, uint32_t version)
{
    auto *manager = static_cast<ClipboardManager *>(data);
    if (strcmp(interface, "wl_data_device_manager") == 0)
    {
        manager->clipboard_manager_ = static_cast<wl_data_device_manager *>(wl_registry_bind(registry, name,
                                                                                             &wl_data_device_manager_interface, version));
    }
}

// Wayland数据设备监听器回调
static void data_device_handle_data_offer(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *id)
{
    // 处理数据提供
    std::cout << "收到Wayland数据提供" << std::endl;
}

static void data_device_handle_enter(void *data, struct wl_data_device *wl_data_device, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *id)
{
    // 处理进入事件
    std::cout << "收到Wayland进入事件" << std::endl;
}

static void data_device_handle_leave(void *data, struct wl_data_device *wl_data_device)
{
    // 处理离开事件
    std::cout << "收到Wayland离开事件" << std::endl;
}

static void data_device_handle_motion(void *data, struct wl_data_device *wl_data_device, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
    // 处理移动事件
    std::cout << "收到Wayland移动事件" << std::endl;
}

static void data_device_handle_drop(void *data, struct wl_data_device *wl_data_device)
{
    // 处理放置事件
    std::cout << "收到Wayland放置事件" << std::endl;
}

static void data_device_handle_selection(void *data, struct wl_data_device *wl_data_device, struct wl_data_offer *id)
{
    // 处理剪贴板选择
    std::cout << "收到Wayland剪贴板选择" << std::endl;
}

// Wayland接口实现
const struct wl_registry_listener ClipboardManager::registry_listener_ = {
    registry_handle_global,
    nullptr // handle_global_remove
};

const struct wl_data_device_listener ClipboardManager::data_device_listener_ = {
    data_device_handle_data_offer,
    data_device_handle_enter,
    data_device_handle_leave,
    data_device_handle_motion,
    data_device_handle_drop,
    data_device_handle_selection};
