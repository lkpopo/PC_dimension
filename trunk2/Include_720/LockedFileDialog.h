#ifndef LOCKEDFILEDIALOG_H
#define LOCKEDFILEDIALOG_H

#include <QFileDialog>
#include <QDir>

class LockedFileDialog : public QFileDialog
{
    Q_OBJECT
public:
    // 构造函数：传入要锁定的路径 + 父窗口
    explicit LockedFileDialog(const QString& lockedPath, QWidget* parent = nullptr)
        : QFileDialog(parent), m_lockedPath(lockedPath)
    {
        // 基础设置：仅选单个已存在的文件（核心修改点1）
        setFileMode(QFileDialog::ExistingFile); // 强制单选，禁用多选
        setAcceptMode(QFileDialog::AcceptOpen); // 打开文件模式

        // 锁定初始目录
        setDirectory(m_lockedPath);
        // 必须禁用原生对话框，否则目录监听和导航限制不生效
        setOption(QFileDialog::DontUseNativeDialog, true);

        // 监听目录变化：强制切回锁定路径（防止用户切换目录）
        connect(this, &QFileDialog::directoryEntered, this, [this](const QString& enteredPath) {
            // 严格锁定：仅允许当前路径是锁定路径（不允许子目录）
            // 若想允许子目录，可改为：if (!enteredPath.startsWith(m_lockedPath))
            if (enteredPath != m_lockedPath) {
                this->setDirectory(m_lockedPath); // 强制切回锁定路径
            }
            });
    }

private:
    QString m_lockedPath; // 锁定的目标路径
};

#endif // LOCKEDFILEDIALOG_H