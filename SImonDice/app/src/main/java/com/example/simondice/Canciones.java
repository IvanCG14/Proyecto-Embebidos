package com.example.simondice;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class Canciones extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Button bt, bt2, bt3, bt4, sal, seg;
        TextView txt;
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_canciones);
        txt = (TextView) findViewById(R.id.textView5);
        bt = (Button) findViewById(R.id.button6);
        bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //1
            }
        });
        bt2 = (Button) findViewById(R.id.button8);
        bt2.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //2
            }
        });
        bt3 = (Button) findViewById(R.id.button9);
        bt3.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //3
            }
        });
        bt4 = (Button) findViewById(R.id.button10);
        bt4.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //4
            }
        });
        sal = (Button) findViewById(R.id.salir);
        sal.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent intent = new Intent(Canciones.this, Principal.class);
                startActivity(intent);
            }
        });
        seg = (Button) findViewById(R.id.seguir);
        seg.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                txt.setText("Escoja una canción");
            }
        });
    }
}