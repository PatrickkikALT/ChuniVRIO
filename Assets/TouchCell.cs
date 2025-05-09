using System;
using UnityEngine;

public class TouchCell : MonoBehaviour {
  [SerializeField] private int opBtn;
  public GameObject player;
  private void OnCollisionEnter(Collision other) {
    if (other.gameObject == player) {
      Debug.Log("Touch");
      SendBtn();
    }
  }

  [ContextMenu("Test")]
  public void SendBtn() {
    ChuniIO.Instance.SendBtnToSharedMemory(opBtn);
  }
}
