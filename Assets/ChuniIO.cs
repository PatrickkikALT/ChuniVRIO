using System;
using System.Runtime.InteropServices;
using System.Text;
using UnityEditor.Rendering;
using UnityEngine;

public class ChuniIO : MonoBehaviour {
  [SerializeField] private string shmemName = "/chuniio";
  [SerializeField] private int size = 1024;

  [DllImport("libc", SetLastError = true)]
  static extern int shm_open(string name, int oflag, uint mode);
  
  [DllImport("libc", SetLastError = true)]
  static extern unsafe int* mmap(int* addr, int length, int prot, int flags, int fd, int offset);
  
  [DllImport("libc", SetLastError = true)]
  static extern unsafe int munmap(int* addr, int* length);
  
  private int _readWrite = 2;
  private int _protectionWrite = 2;
  private int _mapShared = 1;

  public static ChuniIO Instance;

  private void Start() {
    if (Instance == null) Instance = this; else Destroy(this);
    
    SendMessageToSharedMemory("Connected to Memory");
  }

  public unsafe void SendMessageToSharedMemory(string message) {
    int fd = shm_open(shmemName, _readWrite, 0666);
    if (fd == -1) {
      Debug.LogError("Failed to open."); 
      return;
    }
    
    int* ptr = mmap(null, size, _protectionWrite, _mapShared, fd, 0);
    if (ptr == (int*)-1) {
      Debug.LogError("Failed to map.");
    }
    
    byte[] bytes = Encoding.ASCII.GetBytes(message + '\0');
    Marshal.Copy(bytes, 0, (IntPtr)ptr, bytes.Length);
  }
  public unsafe void SendBtnToSharedMemory(int btn) {
    int fd = shm_open(shmemName, _readWrite, 0666);
    if (fd == -1) {
      Debug.LogError("Failed to open."); 
      return;
    }
    
    int* ptr = mmap(null, size, _protectionWrite, _mapShared, fd, 0);
    if (ptr == (int*)-1) {
      Debug.LogError("Failed to map.");
      return;
    }

    string message = "Pressed button: " + btn;
    byte[] bytes = Encoding.ASCII.GetBytes(message + '\0');
    Marshal.Copy(bytes, 0, (IntPtr)ptr, bytes.Length);

    Debug.Log("Wrote to shared memory: " + message);
    
    
    fixed (int* s = &size) {
      munmap(ptr, s);
    }
  }

  void OnDestroy() {
    
  }
}