using System;
using UnityEngine;

public class TouchCell : MonoBehaviour {
  [SerializeField] private int btn;
  public GameObject player;

  void Start() {
    player = GameManager.Instance.GetPlayer();
  }
  private void OnCollisionEnter(Collision other) {
    if (other.gameObject.CompareTag("Cell")) return;
    Debug.Log("Collision");
    GetComponent<Renderer>().material.color = Color.red;
    SendBtn();
  }

  private void OnCollisionExit(Collision other) {
    if (other.transform.root.gameObject == player) {
      GetComponent<Renderer>().material.color = Color.white;
    }
    
  }

  [ContextMenu("Test")]
  public void SendBtn() {
    ChuniIO.SendBtnToIO(btn);
  }
}
