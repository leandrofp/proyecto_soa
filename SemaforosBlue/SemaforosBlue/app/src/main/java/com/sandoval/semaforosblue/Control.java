package com.sandoval.semaforosblue;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.speech.RecognizerIntent;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;


import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * Created by Sandoval Ariel on 12/11/2016.
 */



public class Control extends AppCompatActivity {

    private static final int RECOGNIZE_SPEECH_ACTIVITY = 1;
    String Voz;
    Button btnAvenida, btnCalle, btnDesconectar, btnVoz;
    TextView TextVoz;
    private ProgressDialog progress;
    String address = null;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    private boolean isBtConnected = false;
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Intent newint = getIntent();
        address = newint.getStringExtra(At_ListaDeDispositivos.EXTRA_ADDRESS); //recivimos la mac address obtenida en la actividad anterior

        setContentView(R.layout.at_control);

        btnAvenida = (Button)findViewById(R.id.BtnAvenida);
        btnCalle = (Button)findViewById(R.id.BtnCalle);
        btnDesconectar = (Button)findViewById(R.id.btnDesconectar);
        btnVoz = (Button)findViewById(R.id.BtnVoz);
        TextVoz = (TextView) findViewById(R.id.textVoz);

        new ConnectBT().execute(); //Call the class to connect


        btnAvenida.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                AbrirAvenida();
            }
        });

        btnCalle.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                AbrirCalle();
            }
        });


        btnVoz.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                onClickImgBtnHablar();
            }
        });

        btnDesconectar.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                Disconnect();
            }
        });
    }

    private void Disconnect()
    {
        if (btSocket!=null)
        {
            try
            {
                btSocket.close();
            }
            catch (IOException e)
            { msg("Error");}
        }
        finish();

    }

    private void AbrirCalle()
    {
        if (btSocket!=null)
        {
            try
            {
                btSocket.getOutputStream().write("c".toString().getBytes());
            }
            catch (IOException e)
            {
                msg("Error");
            }
        }
    }

    private void AbrirAvenida()
    {
        if (btSocket!=null)
        {
            try
            {
                btSocket.getOutputStream().write("a".toString().getBytes());
            }
            catch (IOException e)
            {
                msg("Error");
            }
        }
    }

    private void msg(String s)
    {
        Toast.makeText(getApplicationContext(),s,Toast.LENGTH_LONG).show();
    }

    private class ConnectBT extends AsyncTask<Void, Void, Void>  // UI thread
    {
        private boolean ConnectSuccess = true;

        @Override
        protected void onPreExecute()
        {
            progress = ProgressDialog.show(Control.this, "Connecting...", "Please wait!!!");
        }

        @Override
        protected Void doInBackground(Void... devices)
        {
            try
            {
                if (btSocket == null || !isBtConnected)
                {
                    myBluetooth = BluetoothAdapter.getDefaultAdapter();
                    BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);//conectamos al dispositivo y chequeamos si esta disponible
                    btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                    btSocket.connect();
                }
            }
            catch (IOException e)
            {
                ConnectSuccess = false;
            }
            return null;
        }
        @Override
        protected void onPostExecute(Void result)
        {
            super.onPostExecute(result);

            if (!ConnectSuccess)
            {
                msg("Conexión Fallida");
                finish();
            }
            else
            {
                msg("Conectado");
                isBtConnected = true;
            }
            progress.dismiss();
        }
    }


    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        switch (requestCode) {
            case RECOGNIZE_SPEECH_ACTIVITY:

                if (resultCode == RESULT_OK && null != data) {

                    ArrayList<String> speech = data
                            .getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS);

                    String strSpeech2Text = speech.get(0);
                    TextVoz.setText(strSpeech2Text);
                    Voz = strSpeech2Text;
                    Voz.toLowerCase();

                    if (Voz.compareTo("calle") == 0) {
                        AbrirCalle();
                    }

                    if (Voz.compareTo("avenida") == 0){
                        AbrirAvenida();
                    }
                }

                break;
            default:

                break;
        }
    }

    private void onClickImgBtnHablar() {

        Intent intentActionRecognizeSpeech = new Intent(
                RecognizerIntent.ACTION_RECOGNIZE_SPEECH);

        // Configura el Lenguaje (Español-México)
        intentActionRecognizeSpeech.putExtra(
                RecognizerIntent.EXTRA_LANGUAGE_MODEL, "es-MX");
        try {
            startActivityForResult(intentActionRecognizeSpeech,
                    RECOGNIZE_SPEECH_ACTIVITY);
        } catch (ActivityNotFoundException a) {
            Toast.makeText(getApplicationContext(),
                    "Tú dispositivo no soporta el reconocimiento por voz",
                    Toast.LENGTH_SHORT).show();
        }

    }
}
