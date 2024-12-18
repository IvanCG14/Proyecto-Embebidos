package com.example.simondice;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class Juego extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Button bt, bt2, bt3, sal, seg;
        TextView txt;
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_juego);
        txt = (TextView) findViewById(R.id.textView3);
        bt = (Button) findViewById(R.id.button4);
        bt.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //1
            }
        });
        bt2 = (Button) findViewById(R.id.button5);
        bt2.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //2
            }
        });
        bt3 = (Button) findViewById(R.id.button7);
        bt3.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //3
            }
        });
        sal = (Button) findViewById(R.id.salir);
        sal.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                Intent intent = new Intent(Juego.this, Principal.class);
                startActivity(intent);
            }
        });
        seg = (Button) findViewById(R.id.seguir);
        seg.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                txt.setText("Escoja una dificultad");
            }
        });
    }
}