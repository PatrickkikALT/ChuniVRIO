using System;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;

public class ChuniIO : MonoBehaviour
{
  [SerializeField] private string shmemName = "/chuniio";
  [SerializeField] private int size = 1024;

  [DllImport("libc", SetLastError = true)]
  static extern int shm_open(string name, int oflag, uint mode);

  [DllImport("libc", SetLastError = true)]
  static extern int ftruncate(int fd, int length);

  [DllImport("libc", SetLastError = true)]
  static extern IntPtr mmap(IntPtr addr, int length, int prot, int flags, int fd, int offset);

  private int _readWrite = 2;
  private int _protectionWrite = 2;
  private int _mapShared = 1;

  public static ChuniIO Instance;

  private void Start() {
    if (Instance == null) Instance = this; else Destroy(this);
  }

  
  
  public void SendBtnToSharedMemory(int opBtn) {
    int fd = shm_open(shmemName, _readWrite, 0666);
    if (fd == -1) {
      Debug.LogError("Failed to open.");
      return;
    }
    
    IntPtr ptr = mmap(IntPtr.Zero, size, _protectionWrite, _mapShared, fd, 0);
    if (ptr == (IntPtr)(-1)) {
      Debug.LogError("Failed to map.");
      return;
    }

    string message = "Test123" + opBtn;
    byte[] bytes = Encoding.ASCII.GetBytes(message + '\0');
    Marshal.Copy(bytes, 0, ptr, bytes.Length);

    Debug.Log("Wrote to shared memory: " + message);
  }
}